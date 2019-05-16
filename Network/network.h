#ifndef __NETWORK_H
#define __NETWORK_H

#include "stm32f4xx.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#include "usart.h"


#include "esp8266.h"
#include "netlsn.h"
#include "netvoc.h"


#define NET_BUFFER_SIZE		(1024U)


extern uint8_t pucNet_SendBuf[NET_BUFFER_SIZE*2];
extern uint8_t pucNet_ReceiveBuf[NET_BUFFER_SIZE*2];


#define pNET_SEND_BUF0			((uint8_t *)(pucNet_SendBuf))
#define pNET_SEND_BUF1			((uint8_t *)(pucNet_SendBuf + NET_BUFFER_SIZE))

#define pNET_RECEIVE_BUF0			((uint8_t *)(pucNet_ReceiveBuf))
#define pNET_RECEIVE_BUF1			((uint8_t *)(pucNet_ReceiveBuf + NET_BUFFER_SIZE))


static inline void vNet_SendBuf(const char* pbuf, const uint32_t buflen)
{
	xSemaphoreTake(xUsart2_TxMutex, portMAX_DELAY);
	//发送数据
	#define __USE_DMA
#ifdef __USE_DMA
	xSemaphoreTake(xUsart2_DMA_TxBinary, 0);
	vUsart2DMA_TxInit((uint8_t *)pbuf, NULL, buflen);
	vUsart2DMA_TxStart();
	//等待数据发送完毕
	xSemaphoreTake(xUsart2_DMA_TxBinary, portMAX_DELAY);
	vUsart2DMA_TxStop();
#else
	ucUsart2_PutString(pbuf, buflen, portMAX_DELAY);
#endif
	xSemaphoreGive(xUsart2_TxMutex);
}


extern SemaphoreHandle_t xNetStart_Binary;
extern SemaphoreHandle_t xNetEnd_Binary;

void vNet_Task(void *pvParameters);


#endif
