#include "recorder.h"
#include "audio.h"
#include "spi_i2s.h"

#include <string.h>
#include <stdint.h>
#include "wb_vad.h"
#include "ff.h"
#include "wm8978.h"
#include "network.h"



/*
 录制的音频文件是16位量化，8k的采样频率1s 存在 8000 
 次采样 每次采样 占用 2 个 Byte那么一秒的时间产生
 16000 Byte的音频数据,约15.625 k Byte
*/


//--------------------------------------------------------------------
#include "wavplay.h"
static char pwavhead[44] = {0};
//函数说明：填充WAV的文件头
//输入参数：文件头的缓存指针
static void vVoc_FillWavHead(__WaveHeader *pWavhead)
{
	pWavhead->riff.ChunkID = 0x46464952;				//"RIFF"
	pWavhead->riff.ChunkSize = 0;						//还未确定,最后需要计算
	pWavhead->riff.Format = 0x45564157;					//"WAVE"
	pWavhead->fmt.ChunkID = 0x20746D66;					//"fmt "
	pWavhead->fmt.ChunkSize = 16;						//大小为16个字节
	pWavhead->fmt.AudioFormat = 0x01;					//0X01,表示PCM;0X11,表示IMA ADPCM
	pWavhead->fmt.NumOfChannels = 1;					//声道数
	pWavhead->fmt.SampleRate = 8000;					// 采样速率
	pWavhead->fmt.ByteRate = pWavhead->fmt.SampleRate*2;//字节速率=采样率*通道数*(ADC位数/8)
	pWavhead->fmt.BlockAlign = 2;						//块大小=通道数*(ADC位数/8)
	pWavhead->fmt.BitsPerSample = 16;					//16位PCM
	pWavhead->data.ChunkID = 0x61746164;				//"data"
	pWavhead->data.ChunkSize = 0;						//数据大小,还需要计算
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
//单声道音频缓存
static int16_t sRecordAudio[AUDIO_BUFFER_SIZE/2] = {0};

//函数功能：讲双声道PCM转换为单声道PCM
//双声道PCM中,先两个字节是左声道，后两个字节是右声道
//ulBufLen指的是16位的长度（两个字节）
void ChannelFilter(const int16_t *psSrcBuf, \
					int16_t *psDesBuf, \
					const uint32_t ulBufLen, \
					const char Channel)
{
	if (Channel == 'L')
	{
		for(uint32_t i=0,j=0; j<ulBufLen; i++,j+=2)
		{
			psDesBuf[i] = psSrcBuf[j];
		}
	}
	else if (Channel == 'R')
	{
		for(uint32_t i=0,j=0; j<ulBufLen; i++,j+=2)
		{
			psDesBuf[i] = psSrcBuf[j+1];
		}
	}
}
//--------------------------------------------------------------------




//--------------------------------------------------------------------
#include "syslog.h"//用于调试输出
#include "led.h"
//一帧数据设置位144个采样288Byte,一个(AUDIO_BUFFER_SIZE/2)=1152*2,等于8帧
#define FLAME_LEN	(144U)
//一秒8000次采样，144次采样(即一帧)用时18ms

//简单的进行语音端点检测，输入的数据只能是单声道的PCM
//ulBufLen指的是16位的长度（两个字节）
//目前使用的是右声道的参数，如果想用左声道，参数还要调过
static uint8_t ucSimpleVad(const int16_t *psVocBuf, const uint32_t ulBufLen)
{
	static uint8_t usVocSta = 0;		//语音状态机
	static uint32_t usScnt = 0;		//连续静音帧数计数器
	
//------------------------------------------------------------------------------
	uint32_t ulvEnergy = 0;				//短时能量
	uint32_t ulvZerorate = 0;			//短时过零率
	for(uint32_t i=0; i<ulBufLen; i++)
	{
		//计算短时能量
		ulvEnergy += (psVocBuf[i] > 0) ? (psVocBuf[i]) : (-psVocBuf[i]);
		//计算短时过零率
		if(psVocBuf[i]*psVocBuf[i+1] < 0)
		{
			ulvZerorate++;
		}
	}
//------------------------------------------------------------------------------
	
	if (ulvEnergy>150000 && ulvZerorate>15)//这一帧数据达到语音帧的标准
	{
		__LED_OB_ON();
		usVocSta = 1;		//置状态机位语音态
		usScnt = 0;			//静音帧数量统计清零
	}
	else if (ulvEnergy<130000 || ulvZerorate<13)//这一帧数据没达到语音帧的标准
	{
		__LED_OB_OFF();
		//如果状态机还在语音态且静音帧的数量在30帧以内
		if (usScnt < 30 && usVocSta == 1)	//18*30ms
		{
			usScnt++;						//静音帧数量加一
		}
		else
		{
			usVocSta = 0;					//置状态机位静音态
			usScnt = 0;						//静音统计清零
		}
	}
	else//马马虎虎
	{
	}
	//ts_printf("\r\nEner:%d\r\nuZero:%d", ulvEnergy, ulvZerorate);
	return usVocSta;
}

static FIL pcm_fp = {0};
static uint32_t bw = 0;



//录音写入文件
void vRecord_Task(void *pvParameters)
{
	xSemaphoreTake(xNetEnd_Binary, 0);
	xSemaphoreTake(xI2s2DMA_RxBinary, 0);
	
	for (;;)
	{
		xSemaphoreTake(xNetEnd_Binary, portMAX_DELAY);
		WM8978_ADDA_Cfg(0,1);		//开启ADC
		WM8978_Input_Cfg(1,0,0);	//开启输入通道(MIC)
		WM8978_Output_Cfg(0,1);		//开启BYPASS输出
		WM8978_HPvol_Set(0,0);
		WM8978_SPKvol_Set(0);
		WM8978_MIC_Gain(50);		//MIC增益设置(多年实战下来的值)
		WM8978_I2S_Cfg(2,0);		//飞利浦标准,16位数据长度
		vI2s2_ModeInit(I2S_Standard_Phillips, I2S_Mode_MasterTx, I2S_CPOL_Low, I2S_DataFormat_16b, I2S_AudioFreq_8k);
		
		memset(pAUDIO_SEND_BUF0, 0, AUDIO_BUFFER_SIZE);
		memset(pAUDIO_SEND_BUF0, 0, AUDIO_BUFFER_SIZE);
		
		uint32_t ulVocFlameCnt = 0;	//连续语音帧数计数器
		vVoc_FillWavHead((__WaveHeader *)pwavhead);
		//打开缓存文件
		f_open(&pcm_fp, "0:/SmartSpeaker/record", FA_CREATE_ALWAYS | FA_WRITE);
		f_write(&pcm_fp, pwavhead, sizeof(__WaveHeader), &bw);			//写入头数据
		
		for (;;)
		{
			xSemaphoreTake(xI2s2DMA_RxBinary, portMAX_DELAY);
			//获取语音帧的地址
			if (DMA_GetCurrentMemoryTarget(I2S2_RX_STREAM) == 0)
			{
				ChannelFilter((int16_t *)(pAUDIO_RECEIVE_BUF1), \
								sRecordAudio, \
								AUDIO_BUFFER_SIZE/2, \
								'R');
			}
			else
			{
				ChannelFilter((int16_t *)(pAUDIO_RECEIVE_BUF0), \
								sRecordAudio, \
								AUDIO_BUFFER_SIZE/2, \
								'R');
			}
			for (uint32_t fi=0; fi<(AUDIO_BUFFER_SIZE/2/2/FLAME_LEN); fi++)//将数据分帧
			{
				if (ucSimpleVad((int16_t *)(&sRecordAudio[FLAME_LEN*fi]), FLAME_LEN) == 0x01)
				{
					ulVocFlameCnt++;
					f_write(&pcm_fp, \
						(int16_t *)(&sRecordAudio[FLAME_LEN*fi]), \
						FLAME_LEN*2, \
						&bw);
				}
				else if (ulVocFlameCnt > 60)//18*60ms
				{
					goto writevoc;
				}
				else
				{
					ulVocFlameCnt = 0;
					f_lseek(&pcm_fp, sizeof(__WaveHeader));
					f_truncate(&pcm_fp);
				}
			}
		}
writevoc:
		((__WaveHeader *)pwavhead)->riff.ChunkSize = (ulVocFlameCnt)*FLAME_LEN*2+36;	//整个文件的大小-8;
		((__WaveHeader *)pwavhead)->data.ChunkSize = (ulVocFlameCnt)*FLAME_LEN*2;		//数据大小
		f_lseek(&pcm_fp,0);													//偏移到文件头.
		f_write(&pcm_fp, pwavhead, sizeof(__WaveHeader), &bw);				//写入头数据
		f_close(&pcm_fp);
		xSemaphoreGive(xNetStart_Binary);
	}
}


