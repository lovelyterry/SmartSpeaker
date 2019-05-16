#include "bsp_init.h"
#include "usart.h"



//系统公用缓存，用于存放网络监听的数据等
uint8_t PubBuf[PUBBUF_SIZE+2] = {0};


#include "ff.h"


static void mount_fs(void)
{
	static FATFS fs = {0};
	FRESULT res;

	res = f_mount(&fs, "0:", 1);
	if (res == FR_NO_FILESYSTEM)
	{
		//格式化Flash
		res = f_mkfs("0:", 1, 512, NULL, 4096);
	}
	else if (res != FR_OK)
	{
		USART_SendStr(USART1, sizeof("\r\n#[error_msg]: SD卡状态异常，请检查!\r\n") - 1, "\r\n#[error_msg]: SD卡状态异常，请检查!\r\n");
	}
	//创建工作目录
	res = f_mkdir("0:/SmartSpeaker");
}


uint8_t ucBsp_Init(void)
{
	vUsart1_Init(115200);
	vUsart2_Init(115200);
	
	mount_fs();
	return 0;
}

