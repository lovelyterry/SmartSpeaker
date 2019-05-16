//支持16位/24位WAV文件播放
//最高可以支持到192K/24bit的WAV格式.

#include "wavplay.h"
#include "player.h"

#include "ff.h"
#include "wm8978.h"
#include "spi_i2s.h"
#include "audio.h"
#include <string.h>


static __wavctrl wavctrl;		//WAV控制结构体


static uint8_t Wav_GetInfo(const char *pcfile)
{
	FIL fsrc = {0};
	uint32_t byter = 0;
	uint8_t func_res = 0;
	uint8_t *tempBuf = NULL;
	
	ChunkRIFF *riff = NULL;
	ChunkFMT *fmt = NULL;
	ChunkFACT *fact = NULL;
	ChunkDATA *data = NULL;

	tempBuf = pvPortMalloc(512);
	if(tempBuf != NULL)
	{
		f_open(&fsrc, pcfile, FA_READ);
		f_read(&fsrc, tempBuf, 512, &byter);
		riff = (ChunkRIFF *)tempBuf;	//获取RIFF块
		if(riff->Format==0X45564157)	//是WAV文件
		{
			fmt = (ChunkFMT *)(tempBuf+12);	//获取FMT块
			fact = (ChunkFACT *)(tempBuf+12+8+fmt->ChunkSize);//读取FACT块
			if(fact->ChunkID==0X74636166 || fact->ChunkID==0X5453494C)
			{
				wavctrl.datastart=12+8+fmt->ChunkSize+8+fact->ChunkSize;
			}
			else
			{
				wavctrl.datastart=12+8+fmt->ChunkSize;
			}
			data=(ChunkDATA *)(tempBuf+wavctrl.datastart);	//读取DATA块
			if(data->ChunkID==0X61746164)								//解析成功!
			{
				wavctrl.audioformat=fmt->AudioFormat;			//音频格式
				wavctrl.nchannels=fmt->NumOfChannels;			//通道数
				wavctrl.samplerate=fmt->SampleRate;				//采样率
				wavctrl.bitrate=fmt->ByteRate*8;					//得到位速
				wavctrl.blockalign=fmt->BlockAlign;				//块对齐
				wavctrl.bps=fmt->BitsPerSample;						//位数,16/24/32位
				wavctrl.datasize=data->ChunkSize;					//数据块大小
				wavctrl.datastart=wavctrl.datastart+8;		//数据流开始的地方
			}
			else
			{
				func_res = 3;
			}
		}
		else
		{
			func_res = 2;
		}
	}
	else
	{
		func_res = 1;
	}
	return func_res;
}


static FIL wav_fp = {0};		//WAV音乐文件

//stm32是小端模式,Wave文件低地址位上放的是音频数据的低位
//先放左声道，再放右声道，I2S传输时，先传数据的高位，再传低位
//8位数据是无符号的整型数，如果不加处理绘制出来的图形是以0x80为中心上下波动的波形
//16位、24位都是有符号的整形数，这样的数据绘制的图形是以0值为中心上下波动的波形
//32位应该是浮点数
static uint32_t Wav_TransFormot(uint8_t *pucBuf, uint32_t usBufSize)
{
	uint32_t needr = 0;		//需要读取的Byte
	uint32_t byter = 0;		//实际读取的Byte
	uint32_t finar = 0;		//转换后的Byte
	uint8_t *tempBuf = NULL;
	uint8_t *pt = NULL;
	
	tempBuf = pvPortMalloc(usBufSize);
	if(tempBuf != NULL)
	{
		memset(tempBuf, 0, usBufSize);
		
		if(wavctrl.nchannels == 2)//双声道
		{
			if(wavctrl.bps == 16)
			{
				needr = usBufSize;
				f_read(&wav_fp, tempBuf, needr, &finar);
				pt = tempBuf;
				for(uint32_t i=0; i<usBufSize; i++)
				{
					pucBuf[i] = pt[i];
				}
				finar = finar;
			}
			if(wavctrl.bps == 24)
			{
				needr = (usBufSize/4)*3;
				f_read(&wav_fp, tempBuf, needr, &finar);
				pt = tempBuf;
				for(uint32_t i=0; i<usBufSize; )
				{
					pucBuf[i++] = pt[1];
					pucBuf[i] = pt[2]; 
					i += 2;
					pucBuf[i++] = pt[0];
					pt += 3;
				} 
				finar = (finar*4)/3;
			}
		}
		//单声道，调整为双声道数据进行播放
		else
		{
			if(wavctrl.bps == 16)
			{
				needr = usBufSize/2;
				f_read(&wav_fp, tempBuf, needr, &byter);
				pt = tempBuf;
				for(uint32_t i=0; i<usBufSize; i+=4)
				{
					pucBuf[i+0] = pt[0];
					pucBuf[i+1] = pt[1];
					pucBuf[i+2] = pt[0];
					pucBuf[i+3] = pt[1];
					pt += 2;
				}
				finar = byter*2;
			}
		}
		if(finar < usBufSize)//不够数据了,补充0
		{
			for(uint32_t i = byter; i<usBufSize-byter; i++)
			{
				pucBuf[i] = 0;
			}
		}
	}
	vPortFree(tempBuf);
	
	return finar;
}


//返回值:
//0,成功;
//1,内存分配失败;
//2,操作文件失败;
//3,非支持的WAV文件.
uint8_t Wav_Play(const char *pcfile)
{
	uint8_t func_res = 0;
	uint32_t ulFill = 0;
	
	func_res = Wav_GetInfo(pcfile);
	if(func_res == 0)
	{
		if (wavctrl.bps == 16)
		{
			WM8978_ADDA_Cfg(1,0);		//开启DAC
			WM8978_Input_Cfg(0,0,0);	//关闭输入通道
			WM8978_Output_Cfg(1,0);		//开启DAC输出
			WM8978_HPvol_Set(ucLHPvol,ucRHPvol);
			WM8978_SPKvol_Set(ucSPKvol);
			WM8978_MIC_Gain(0);
			WM8978_I2S_Cfg(2,0);		//设置I2S接口模式
			vI2s2_ModeInit(I2S_Standard_Phillips, I2S_Mode_MasterTx, I2S_CPOL_Low, I2S_DataFormat_16b, wavctrl.samplerate);
		}
		else if (wavctrl.bps == 24)
		{
			WM8978_ADDA_Cfg(1,0);		//开启DAC
			WM8978_Input_Cfg(0,0,0);	//关闭输入通道
			WM8978_Output_Cfg(1,0);		//开启DAC输出
			WM8978_HPvol_Set(ucLHPvol,ucRHPvol);
			WM8978_SPKvol_Set(ucSPKvol);
			WM8978_MIC_Gain(0);
			WM8978_I2S_Cfg(2,2);		//设置I2S接口模式
			vI2s2_ModeInit(I2S_Standard_Phillips, I2S_Mode_MasterTx, I2S_CPOL_Low, I2S_DataFormat_24b, wavctrl.samplerate);
		}
		else
		{
			//暂不支持这种位数
			func_res = 3;
		}
		
		f_open(&wav_fp, pcfile, FA_READ);
		f_lseek(&wav_fp, wavctrl.datastart);
		for(;;)
		{
			xSemaphoreTake(xI2s2DMA_TxBinary, portMAX_DELAY);
			if (DMA_GetCurrentMemoryTarget(I2S2_TX_STREAM) == 0)
			{
				ulFill = Wav_TransFormot(pAUDIO_SEND_BUF1, AUDIO_BUFFER_SIZE);
			}
			else
			{
				ulFill = Wav_TransFormot(pAUDIO_SEND_BUF0, AUDIO_BUFFER_SIZE);
			}
			if(ulFill != AUDIO_BUFFER_SIZE) //填充的数量和缓存大小不同，播放结束
			{
				break;
			}
		}
		xSemaphoreTake(xI2s2DMA_TxBinary, portMAX_DELAY);
		memset(pAUDIO_SEND_BUF0, 0, AUDIO_BUFFER_SIZE);
		memset(pAUDIO_SEND_BUF1, 0, AUDIO_BUFFER_SIZE);
		memset(pAUDIO_RECEIVE_BUF0, 0, AUDIO_BUFFER_SIZE);
		memset(pAUDIO_RECEIVE_BUF1, 0, AUDIO_BUFFER_SIZE);
		f_close(&wav_fp);
	}
	return func_res;
}


