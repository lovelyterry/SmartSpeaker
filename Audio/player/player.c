#include "player.h"
#include "wm8978.h"
#include "ff.h"

#include <string.h>

#include "wavplay.h"
#include "mp3play.h"



void vPlayer_Task(void *pvParameters)
{
	if (pvParameters != NULL)
	{
		if(strstr(pvParameters, ".mp3") != NULL)
		{
			MP3_Play(pvParameters);
		}
		else if(strstr(pvParameters, ".wav") != NULL)
		{
			Wav_Play(pvParameters);
		}
	}
	else
	{
		char *pcPath = NULL;
		
		FILINFO finfo = {0};
		DIR dirs = {0};
		
		pcPath = pvPortMalloc(MAX_DIR_SIZE);
		if (pcPath != NULL)
		{
			f_opendir(&dirs, MUSIC_FOLDER);
			while(f_readdir(&dirs, &finfo) == FR_OK && finfo.fname[0] != 0)
			{
				if(finfo.fattrib & AM_ARC)	//如果文件属性为档案
				{
					memset(pcPath, 0, MAX_DIR_SIZE);
					if(strstr(finfo.fname, ".mp3") != NULL)
					{
						strcpy(pcPath, MUSIC_FOLDER);
						strcat(pcPath, "/");
						strcat(pcPath, finfo.fname);
						MP3_Play(pcPath);
					}
					else if(strstr(finfo.fname, ".wav") != NULL)
					{
						strcpy(pcPath, MUSIC_FOLDER);
						strcat(pcPath, "/");
						strcat(pcPath, finfo.fname);
						Wav_Play(pcPath);
					}
				}
			}
		}
		vPortFree(pcPath);
	}
	vTaskDelete(NULL);
}


