#ifndef __I2S_BUS_H
#define __I2S_BUS_H

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define I2S2_RX_STREAM		DMA1_Stream3
#define I2S2_TX_STREAM		DMA1_Stream4


extern SemaphoreHandle_t xI2s2DMA_TxBinary;
extern SemaphoreHandle_t xI2s2DMA_RxBinary;


void vI2s2_GpioInit(void);
void vI2s2_ClockInit(void);
void vI2s2_ModeInit(uint16_t I2S_Standard, \
				uint16_t I2S_Mode, \
				uint16_t I2S_CPOL, \
				uint16_t I2S_DataFormat, \
				uint32_t I2S_AudioFreq\
				);

void vI2s2_BinaryInit(void);


void vI2s2DMA_TxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum);
void vI2s2DMA_TxStart(void);
void vI2s2DMA_TxStop(void);
void vI2s2DMA_RxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum);
void vI2s2DMA_RxStart(void);
void vI2s2DMA_RxStop(void);
//----------------------------------------------------------------------------------------

void SPI1_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t TxData);


#endif

