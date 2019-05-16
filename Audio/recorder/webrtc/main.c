#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "wb_vad.h"


//音频VAD输入缓冲区,webrtc支持输入的音频是32bit浮点pcm,
//而我录制的音频是16bit的其中需要一个转换的过程。
float pfIndata[FRAME_LEN] = {0};
//文件读取缓冲区指针
uint16_t pusPCMbuf[FRAME_LEN] = {0};

//缓存输出的语音片段的名字
char cOfname[128] = {0};
//用于指示记录的状态,0表示没有在记录，1表示正在记录中
uint8_t ucVocsta = 0;
//用于记录静音帧的数量，当静音帧的数量超过一定的额度后，判定语音段结束
uint16_t usScnt = 0;
//用于记录该文件语音段的数量，也就是输出文件的数量
uint16_t usFcnt = 0;
//一个VAD结构体指针变量
VadVars *xState = NULL;



int main(int argc, char* argv[])
{
	FILE *iwav_fp = NULL;
	FILE *opcm_fp = NULL;
	
	if (argc != 2)
	{
		printf("usage:./vad wav_file\n");
		return -1;
	}
	
	iwav_fp = fopen(argv[1], "rb");
	if (iwav_fp == NULL)
	{
		printf("file open filed!\n");
		return -1;
	}
	
	//动态分配VAD结构体的内存，并初始化
	wb_vad_init(&(xState));
	/*一般的WAV文件有44byte的文件头，剩下的是pcm格式的音频数据，跳过这44byte*/
	fseek(iwav_fp, 44, SEEK_SET);
	
	while (fread(pusPCMbuf, sizeof(uint16_t), FRAME_LEN, iwav_fp) > 0)
	{
		//判断
		for (int i = 0; i<FRAME_LEN; i++)
		{
			//无符号short隐式自动转换为float
			pfIndata[i] = pusPCMbuf[i];
			//十六位最多表示无符号的0到65535,音频数据因该是有符号的
			//如果大于65535/2,就说明是个负数了,我们将其转为负数值.
			if (pfIndata[i] > 65535/2)
			{
				pfIndata[i] = pfIndata[i]-65536;
			}
		}
		/*VAD Decision, 1 = speech, 0 = noise*/
		if (wb_vad(xState, pfIndata) == 1)
		{
			//静音帧数量统计清零
			usScnt = 0;
			//置状态机位语音态
			ucVocsta = 1;
		}
		else
		{
			//如果状态机还在语音态且静音帧的数量在120帧以内
			if (usScnt < 120 && ucVocsta == 1)
			{
				//静音帧数量加一
				usScnt++;
				//直接写入文件
				goto writef;
			}
			else
			{
				//置状态机位静音态
				ucVocsta = 0;
				//静音统计清零
				usScnt = 0;
			}
		}
		//如果状态机是语音态，pcm的文件指针为空，说明是第一帧，我们创建一个新文件。
		if (ucVocsta == 1 && opcm_fp == NULL)
		{
			sprintf(cOfname, "voc.%d.wav", usFcnt);
			opcm_fp = fopen(cOfname, "wb");
		}
		//如果状态机是静音态，而且文件指针不为空，说明语音段已经结束了，我们关闭该文件
		if (ucVocsta == 0 && opcm_fp != NULL)
		{
			fclose(opcm_fp);
			usFcnt++;
			opcm_fp = NULL;
		}
writef:
		if (opcm_fp != NULL && ucVocsta == 1)
		{
			fwrite(pusPCMbuf, sizeof(uint16_t), FRAME_LEN, opcm_fp);
		}
	}
	//释放动态分配的内存
	wb_vad_exit(&xState);
	fclose(iwav_fp);
	
	return 0;
}



