#include "netcmd.h"
#include <string.h>
#include "wavplay.h"
#include "syslog.h"


static const netcmd_t netcmd_array[];

//函数说明：
//再命令列表中寻找对应的关键字，找到了就执行相应的操作
//返回值：0，找到并执行了相应的函数
//1，没有匹配到关键的字
//2，传入的参数不正确
uint8_t ucParseWord(uint8_t argc, char **argv)
{
	uint8_t sameflag = 0;
	uint8_t runres = 0;
	
	if (argc > 0 && argv[0] != NULL)
	{
		for (uint32_t i=0; netcmd_array[i].cmd_func != NULL; i++)
		{
			for (uint8_t j = 0; ; j++)
			{
				//如果到了关键字数组的结尾
				if (netcmd_array[i].key_word[j] == NULL)
				{
					//找到的关键字大于0个
					if (sameflag == 1)
					{
						Wav_Play("0:/SmartSpeaker/ansok");
						if ((netcmd_array[i].cmd_func) != NULL)
						{
							runres = (netcmd_array[i].cmd_func)(argc, argv);
							if (runres == 0)
							{
								Wav_Play("0:/SmartSpeaker/setok");
							}
							else
							{
								Wav_Play("0:/SmartSpeaker/nofuc");
							}
							return runres;
						}
					}
					//关键字数组为空或没有找到关键字
					else
					{
						break;
					}
				}
				else if (strstr((char *)(&argv[0]), netcmd_array[i].key_word[j]) != NULL)//没有找到关键字
				{
					sameflag = 1;//找到关键字了
				}
				else
				{
					
				}
			}
		}
		Wav_Play("0:/SmartSpeaker/inko");
		return 0xFF;
	}
	else
	{
		return 0xFE;
	}
}


//---------------------------------------------------------------------------------
//{
#include "led.h"

static uint8_t prvNetCmd_Light(uint8_t argc, char **argv)
{
	(void)argc;
	uint8_t light_sta = 0xff;
	
	if (strstr((char *)(&argv[0]), "开") != NULL)
	{
		light_sta = 0x01;
	}
	else if (strstr((char *)(&argv[0]), "关") != NULL)
	{
		light_sta = 0x00;
	}
	
	if (strstr((char *)(&argv[0]), "红") != NULL)
	{
		if (light_sta == 0x01)
		{
			__LED_EX_R_ON();
		}
		else if (light_sta == 0x00)
		{
			__LED_EX_R_OFF();
		}
	}
	
	if (strstr((char *)(&argv[0]), "绿") != NULL)
	{
		if (light_sta == 0x01)
		{
			__LED_EX_G_ON();
		}
		else if (light_sta == 0x00)
		{
			__LED_EX_G_OFF();
		}
	}
	
	if (strstr((char *)(&argv[0]), "蓝") != NULL)
	{
		if (light_sta == 0x01)
		{
			__LED_EX_B_ON();
		}
		else if (light_sta == 0x00)
		{
			__LED_EX_B_OFF();
		}
	}
	return 0;
}
//}
//---------------------------------------------------------------------------------





//---------------------------------------------------------------------------------
//{
//语音控制播放音乐
#include "wavplay.h"
#include "mp3play.h"
#include "ff.h"

static FILINFO finfo = {0};
static DIR dirs = {0};
static char pcPath[256] = {0};

#include "urlencode.h"
static char musickey[256] = {0};
static char temp[256] = {0};

static uint8_t prvNetCmd_Play(uint8_t argc, char **argv)
{
	uint8_t word_same_cnt = 0;
	char gbk_word[2] ={0};
	
	(void)argc;
	uint8_t unilen = 0;		//unicode可能中间包含有0x00，所以不能strlen;
	
	//网络传过来的文字是utf-8的，需要与本地文字比较，则要转换为gbk
	unilen = ulUtf_8toUnicode((char *)(&argv[0]), strlen((char *)(&argv[0])), temp);
	UnicodetoGbk(temp, unilen, musickey);
	
	f_opendir(&dirs, "0:/music");
	while(f_readdir(&dirs, &finfo) == FR_OK && finfo.fname[0] != 0)
	{
		if(finfo.fattrib & AM_ARC)	//如果文件属性为档案
		{
			word_same_cnt = 0;
			for (uint32_t i=0; i<(strlen(finfo.fname)); i+=2)
			{
				memset(gbk_word, 0 ,sizeof(gbk_word));
				strncpy(gbk_word, finfo.fname+i, 2);
				if (strstr(musickey, gbk_word) != NULL)
				{
					word_same_cnt++;
				}
			}
			if(word_same_cnt > (strlen(finfo.fname)-4)/2/2)
			{
				if(strstr(finfo.fname, ".mp3") != NULL)
				{
					strcpy(pcPath, "0:/music");
					strcat(pcPath, "/");
					strcat(pcPath, finfo.fname);
					exucLED_Sta = 1;
					MP3_Play(pcPath);
					exucLED_Sta = 0;
					break;
				}
				else if(strstr(finfo.fname, ".wav") != NULL)
				{
					strcpy(pcPath, "0:/music");
					strcat(pcPath, "/");
					strcat(pcPath, finfo.fname);
					exucLED_Sta = 1;
					Wav_Play(pcPath);
					exucLED_Sta = 0;
					break;
				}
			}
		}
	}
	return 0;
}
//---------------------------------------------------------------------------------








static const netcmd_t netcmd_array[] =
{
	{prvNetCmd_Play, {"放", NULL}},
	{prvNetCmd_Light, {"灯", "登", NULL}},
	
	{NULL, {NULL}}
};




