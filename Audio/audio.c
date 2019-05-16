//初始化WM8978，开启I2S收发双缓存的DMA


#include "audio.h"

#include "spi_i2s.h"
#include "wm8978.h"
#include "syslog.h"
#include <string.h>

#include "player.h"
#include "recorder.h"



uint8_t pucAudio_SendBuf[AUDIO_BUFFER_SIZE*2] = {0};
uint8_t pucAudio_ReceiveBuf[AUDIO_BUFFER_SIZE*2] = {0};


uint8_t ucLHPvol = LHP_DEF;
uint8_t ucRHPvol = RHP_DEF;
uint8_t ucSPKvol = SPK_DEF;



void vAudio_Task(void *pvParameters)
{
	if (WM8978_Init() == 1)
	{
		ts_printf("\r\n#[error_msg]: WM8978状态异常，请检查!\r\n");
	}
	vI2s2_GpioInit();
	vI2s2_ClockInit();
	vI2s2_BinaryInit();
	
	memset(pAUDIO_SEND_BUF0, 0, AUDIO_BUFFER_SIZE);
	memset(pAUDIO_SEND_BUF1, 0, AUDIO_BUFFER_SIZE);
	memset(pAUDIO_RECEIVE_BUF0, 0, AUDIO_BUFFER_SIZE);
	memset(pAUDIO_RECEIVE_BUF1, 0, AUDIO_BUFFER_SIZE);
	//因为DMA设置的传输是以16位为一个单元的，而我们定义的缓存是8位的单元，所以再除以2
	vI2s2DMA_TxInit(pAUDIO_SEND_BUF0, pAUDIO_SEND_BUF1, AUDIO_BUFFER_SIZE/2);
	vI2s2DMA_RxInit(pAUDIO_RECEIVE_BUF0, pAUDIO_RECEIVE_BUF1, AUDIO_BUFFER_SIZE/2);
	
	vI2s2DMA_TxStart();
	vI2s2DMA_RxStart();
	
	xTaskCreate(vRecord_Task, \
				"Record", \
				(configMINIMAL_STACK_SIZE*3), \
				NULL, \
				1, \
				NULL);
	vTaskDelete(NULL);
}




