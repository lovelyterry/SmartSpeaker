#ifndef __USART_H
#define	__USART_H

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define USART2_RX_STREAM		DMA1_Stream5
#define USART2_TX_STREAM		DMA1_Stream6



extern QueueHandle_t xUsart1_TxQueue; 
extern QueueHandle_t xUsart1_RxQueue;

extern SemaphoreHandle_t xUsart1_IDLE_Binary;
extern SemaphoreHandle_t xUsart1_DMA_TxBinary;
extern SemaphoreHandle_t xUsart1_DMA_RxBinary;

extern SemaphoreHandle_t xUsart1_TxMutex;
extern SemaphoreHandle_t xUsart1_RxMutex;


void vUsart1_Init(const uint32_t ulBaudRate);
uint8_t ucUsart1_GetChar(char *pcRxedChar, const TickType_t xBlockTime);
uint8_t ucUsart1_PutChar(const char cOutChar, const TickType_t xBlockTime);
uint8_t ucUsart1_PutString(const char *const pcString, const uint32_t ulStringLength, const TickType_t xBlockTime);




//窃听模式：开启后会将串口2的数据同步发送到串口1(DMA传输的数据无效)
//注意：数据窃听不具有线程保护功能
//注意：串口2的波特率远大于串口1且数据密集时,窃听会有数据丢失
#define __TAPPING_MODE
#define __TAPPING_DEFAULT	(0U)

#ifdef __TAPPING_MODE
extern uint8_t ucTapping;
#endif

extern QueueHandle_t xUsart2_TxQueue; 
extern QueueHandle_t xUsart2_RxQueue;

extern SemaphoreHandle_t xUsart2_IDLE_Binary;
extern SemaphoreHandle_t xUsart2_DMA_TxBinary;
extern SemaphoreHandle_t xUsart2_DMA_RxBinary;

extern SemaphoreHandle_t xUsart2_TxMutex;
extern SemaphoreHandle_t xUsart2_RxMutex;


void vUsart2_Init(const uint32_t ulBaudRate);
uint8_t ucUsart2_GetChar(char *pcRxedChar, const TickType_t xBlockTime);
uint8_t ucUsart2_PutChar(const char cOutChar, const TickType_t xBlockTime);
uint8_t ucUsart2_PutString(const char *const pcString, const uint32_t ulStringLength, const TickType_t xBlockTime);


void vUsart2DMA_TxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum);
void vUsart2DMA_RxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum);
void vUsart2DMA_TxStart(void);
void vUsart2DMA_RxStart(void);
void vUsart2DMA_TxStop(void);
void vUsart2DMA_RxStop(void);


#define __IO_2USART


void USART_SendByte(USART_TypeDef *USARTx, char cData);
char USART_ReceiveByte(USART_TypeDef *USARTx);
void USART_SendStr(USART_TypeDef *USARTx, uint32_t ulLen, const char *pcData);

#endif /* __USART_H */
