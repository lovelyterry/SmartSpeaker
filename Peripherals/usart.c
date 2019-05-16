#include "usart.h"




#define USART1_TX_QUEUE_SIZE		(0xFFU)
#define USART1_RX_QUEUE_SIZE		(0xFFU)


QueueHandle_t xUsart1_TxQueue = NULL; 
QueueHandle_t xUsart1_RxQueue = NULL;

SemaphoreHandle_t xUsart1_IDLE_Binary = NULL;
SemaphoreHandle_t xUsart1_DMA_TxBinary = NULL;
SemaphoreHandle_t xUsart1_DMA_RxBinary = NULL;

SemaphoreHandle_t xUsart1_TxMutex = NULL;
SemaphoreHandle_t xUsart1_RxMutex = NULL;

static void prvUsart1_GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10, GPIO_AF_USART1);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}


static void prvUsart1_ModeInit(const uint32_t ulBaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}


static void prvUsart1_NvicInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0A;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


static void prvUsart1_QueueInit(void)
{
	static uint8_t Usart1_QueueSetFlag = 0x00;
	
	if (Usart1_QueueSetFlag == 0x00)
	{
		/* Create the queue of chars that are waiting to be sent to console. */
		xUsart1_TxQueue = xQueueCreate(USART1_TX_QUEUE_SIZE, sizeof(char));
		/* Create the queue used to hold characters received from console. */
		xUsart1_RxQueue = xQueueCreate(USART1_RX_QUEUE_SIZE, sizeof(char));
		if ((xUsart1_TxQueue && xUsart1_RxQueue) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		Usart1_QueueSetFlag = 0x01;
	}
}


static void prvUsart1_BinaryInit(void)
{
	static uint8_t Usart1_BinarySetFlag = 0x00;
	
	if (Usart1_BinarySetFlag == 0x00)
	{
		vSemaphoreCreateBinary(xUsart1_IDLE_Binary);
		vSemaphoreCreateBinary(xUsart1_DMA_TxBinary);
		vSemaphoreCreateBinary(xUsart1_DMA_RxBinary);
		if ((xUsart1_IDLE_Binary && xUsart1_DMA_RxBinary && xUsart1_DMA_RxBinary) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		Usart1_BinarySetFlag = 0x01;
	}
}

static void prvUsart1_MutexInit(void)
{
	static uint8_t Usart1_MutexSetFlag = 0x00;
	
	if (Usart1_MutexSetFlag == 0x00)
	{
		xUsart1_TxMutex = xSemaphoreCreateMutex();
		xUsart1_RxMutex = xSemaphoreCreateMutex();
		if ((xUsart1_TxMutex && xUsart1_RxMutex) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		Usart1_MutexSetFlag = 0x01;
	}
}

void vUsart1_Init(const uint32_t ulBaudRate)
{
	prvUsart1_GpioInit();
	prvUsart1_ModeInit(ulBaudRate);
	prvUsart1_NvicInit();
	
	prvUsart1_QueueInit();
	prvUsart1_BinaryInit();
	prvUsart1_MutexInit();
}


uint8_t ucUsart1_GetChar(char *pcRxedChar, const TickType_t xBlockTime)
{
	if (xQueueReceive(xUsart1_RxQueue, pcRxedChar, xBlockTime) == pdPASS)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


uint8_t ucUsart1_PutChar(const char cOutChar, const TickType_t xBlockTime)
{
	if (xQueueSend(xUsart1_TxQueue, &cOutChar, xBlockTime) == pdPASS)
	{
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		return 0;
	}
	else
	{
		return 1;
	}
}


uint8_t ucUsart1_PutString(const char *const pcString, const uint32_t ulStringLength, const TickType_t xBlockTime)
{
	uint8_t ucPutRes = 0;

	for (uint32_t i = 0; i < ulStringLength; i++)
	{
		ucPutRes &= ucUsart1_PutChar(pcString[i], xBlockTime);
	}
	return ucPutRes;
}


/*------------------------------------中断函数------------------------------------------*/
void USART1_IRQHandler(void)
{
	BaseType_t HigherPriorityTaskWoken = pdTRUE;
	/*防止编译器优化*/
	volatile uint32_t temp;
	char cChar;
	
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
		/* The interrupt was caused by the THR becoming empty.  Are there any
		more characters to transmit? */
		if (xQueueReceiveFromISR(xUsart1_TxQueue, &cChar, &HigherPriorityTaskWoken) == pdPASS)
		{
			/* A character was retrieved from the buffer so can be sent to the
			THR now. */
			USART_SendData(USART1, (uint8_t)cChar);
		}
		else
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
	}
	
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		cChar = (uint8_t)USART_ReceiveData(USART1);
		xQueueSendFromISR(xUsart1_RxQueue, &cChar, &HigherPriorityTaskWoken);
	}
	
	/*总线空闲中断，初始化完毕后会进入一次，发送完毕后会进入一次，接受完毕后也会进入一次*/
	if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)//总线空闲中断，表示数据已经发送完成
	{
		//IDLE 该位由软件序列清零（读入 USART_SR 寄存器，然后读入 USART_DR 寄存器）。
		temp = USART1->SR;
		temp = USART1->DR;
		temp = temp;
		xSemaphoreGiveFromISR(xUsart1_IDLE_Binary, &HigherPriorityTaskWoken);
	}
}

//TODO:暂时没有用到DMA,如果需要用到，请参考串口2代码

/*--------------------------------------------------------------------------------------*/









#define USART2_TX_QUEUE_SIZE		(0xFFU)
#define USART2_RX_QUEUE_SIZE		(0xFFU)


QueueHandle_t xUsart2_TxQueue = NULL; 
QueueHandle_t xUsart2_RxQueue = NULL;

SemaphoreHandle_t xUsart2_IDLE_Binary = NULL;
SemaphoreHandle_t xUsart2_DMA_TxBinary = NULL;
SemaphoreHandle_t xUsart2_DMA_RxBinary = NULL;

SemaphoreHandle_t xUsart2_TxMutex = NULL;
SemaphoreHandle_t xUsart2_RxMutex = NULL;


static void prvUsart2_GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_USART2);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}


static void prvUsart2_ModeInit(const uint32_t ulBaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_Cmd(USART2, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_TC);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}


static void prvUsart2_NvicInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0A;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


static void prvUsart2_QueueInit(void)
{
	static uint8_t Usart2_QueueSetFlag = 0x00;
	
	if (Usart2_QueueSetFlag == 0x00)
	{
		/* Create the queue of chars that are waiting to be sent to console. */
		xUsart2_TxQueue = xQueueCreate(USART2_TX_QUEUE_SIZE, sizeof(char));
		/* Create the queue used to hold characters received from console. */
		xUsart2_RxQueue = xQueueCreate(USART2_RX_QUEUE_SIZE, sizeof(char));
		if ((xUsart2_TxQueue && xUsart2_RxQueue) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		Usart2_QueueSetFlag = 0x01;
	}
}


static void prvUsart2_BinaryInit(void)
{
	static uint8_t Usart2_BinarySetFlag = 0x00;
	
	if (Usart2_BinarySetFlag == 0x00)
	{
		vSemaphoreCreateBinary(xUsart2_IDLE_Binary);
		vSemaphoreCreateBinary(xUsart2_DMA_TxBinary);
		vSemaphoreCreateBinary(xUsart2_DMA_RxBinary);
		if ((xUsart2_IDLE_Binary && xUsart2_DMA_RxBinary && xUsart2_DMA_RxBinary) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		Usart2_BinarySetFlag = 0x01;
	}
}


static void prvUsart2_MutexInit(void)
{
	static uint8_t Usart2_MutexSetFlag = 0x00;
	
	if (Usart2_MutexSetFlag == 0x00)
	{
		xUsart2_TxMutex = xSemaphoreCreateMutex();
		xUsart2_RxMutex = xSemaphoreCreateMutex();
		if ((xUsart2_TxMutex && xUsart2_RxMutex) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		Usart2_MutexSetFlag = 0x01;
	}
}


void vUsart2_Init(const uint32_t ulBaudRate)
{
	prvUsart2_GpioInit();
	prvUsart2_ModeInit(ulBaudRate);
	prvUsart2_NvicInit();
	
	prvUsart2_QueueInit();
	prvUsart2_BinaryInit();
	prvUsart2_MutexInit();
}


uint8_t ucUsart2_GetChar(char *pcRxedChar, const TickType_t xBlockTime)
{
	if (xQueueReceive(xUsart2_RxQueue, pcRxedChar, xBlockTime) == pdPASS)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


uint8_t ucUsart2_PutChar(const char cOutChar, const TickType_t xBlockTime)
{
	if (xQueueSend(xUsart2_TxQueue, &cOutChar, xBlockTime) == pdPASS)
	{
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
		return 0;
	}
	else
	{
		return 1;
	}
}


uint8_t ucUsart2_PutString(const char *const pcString, const uint32_t ulStringLength, const TickType_t xBlockTime)
{
	uint8_t ucPutRes = 0;

	for (uint32_t i = 0; i < ulStringLength; i++)
	{
		ucPutRes &= ucUsart2_PutChar(pcString[i], xBlockTime);
	}
	return ucPutRes;
}


void vUsart2DMA_TxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(USART2_TX_STREAM);
	while (DMA_GetCmdStatus(USART2_TX_STREAM) != DISABLE)
	{
		;
	}
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(USART2->DR));
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)pucBufA;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = usNum;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//为了防止DMA无脑循环发送数据，还是设置成单次传输
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(USART2_TX_STREAM, &DMA_InitStructure);
	
	if (pucBufB != NULL)
	{
		//使用双缓冲时会自动设置为循环模式(DMA_Mode_Circular)
		DMA_DoubleBufferModeConfig(USART2_TX_STREAM, (uint32_t)pucBufB, DMA_Memory_0);
		DMA_DoubleBufferModeCmd(USART2_TX_STREAM, ENABLE);
	}
	
	DMA_ITConfig(USART2_TX_STREAM, DMA_IT_TC, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0A;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void vUsart2DMA_RxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(USART2_RX_STREAM);
	while (DMA_GetCmdStatus(USART2_RX_STREAM) != DISABLE)
	{
		;
	}

	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(USART2->DR));
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)pucBufA;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = usNum;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(USART2_RX_STREAM, &DMA_InitStructure);
	
	if (pucBufB != NULL)
	{
		DMA_DoubleBufferModeConfig(USART2_RX_STREAM, (uint32_t)pucBufB, DMA_Memory_0);
		DMA_DoubleBufferModeCmd(USART2_RX_STREAM, ENABLE);
	}
	
	DMA_ITConfig(USART2_RX_STREAM, DMA_IT_TC, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0A;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void vUsart2DMA_TxStart(void)
{
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(USART2_TX_STREAM, ENABLE);
}

void vUsart2DMA_RxStart(void)
{
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(USART2_RX_STREAM, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
}

void vUsart2DMA_TxStop(void)
{
	USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);
	DMA_Cmd(USART2_TX_STREAM, DISABLE);
}

void vUsart2DMA_RxStop(void)
{
	USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
	DMA_Cmd(USART2_RX_STREAM, DISABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
}


/*------------------------------------中断函数------------------------------------------*/
#ifdef __TAPPING_MODE
uint8_t ucTapping = __TAPPING_DEFAULT;
#endif
void USART2_IRQHandler(void)
{
	BaseType_t HigherPriorityTaskWoken = pdTRUE;
	/*防止编译器优化*/
	volatile uint32_t temp;
	char cChar;
	
	
	if (USART_GetITStatus(USART2, USART_IT_TXE) == SET)
	{
		/* The interrupt was caused by the THR becoming empty.  Are there any
		more characters to transmit? */
		if (xQueueReceiveFromISR(xUsart2_TxQueue, &cChar, &HigherPriorityTaskWoken) == pdPASS)
		{
			/* A character was retrieved from the buffer so can be sent to the
			THR now. */
			USART_SendData(USART2, (uint8_t)cChar);
		}
		else
		{
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		}
	}
	
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		cChar = (uint8_t)USART_ReceiveData(USART2);
		xQueueSendFromISR(xUsart2_RxQueue, &cChar, &HigherPriorityTaskWoken);
#ifdef __TAPPING_MODE
		if(ucTapping == 0x01)
		{
			xQueueSendFromISR(xUsart1_TxQueue, &cChar, &HigherPriorityTaskWoken);
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		}
#endif
	}
	
	/*总线空闲中断，初始化完毕后会进入一次，发送完毕后会进入一次，接受完毕后也会进入一次*/
	if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)//总线空闲中断，表示数据已经发送完成
	{
		//IDLE 该位由软件序列清零（读入 USART_SR 寄存器，然后读入 USART_DR 寄存器）。
		temp = USART2->SR;
		temp = USART2->DR;
		temp = temp;
		xSemaphoreGiveFromISR(xUsart2_IDLE_Binary, &HigherPriorityTaskWoken);
	}
}


void DMA1_Stream5_IRQHandler(void)
{
	BaseType_t HigherPriorityTaskWoken = pdTRUE;
	
	if (DMA_GetITStatus(USART2_RX_STREAM, DMA_IT_TCIF5) == SET)
	{
		DMA_ClearITPendingBit(USART2_RX_STREAM, DMA_IT_TCIF5);
		xSemaphoreGiveFromISR(xUsart2_DMA_RxBinary, &HigherPriorityTaskWoken);
	}
}


void DMA1_Stream6_IRQHandler(void)
{
	BaseType_t HigherPriorityTaskWoken = pdTRUE;
	
	if (DMA_GetITStatus(USART2_TX_STREAM, DMA_IT_TCIF6) == SET)
	{
		DMA_ClearITPendingBit(USART2_TX_STREAM, DMA_IT_TCIF6);
		xSemaphoreGiveFromISR(xUsart2_DMA_TxBinary, &HigherPriorityTaskWoken);
	}
}
/*--------------------------------------------------------------------------------------*/



void USART_SendByte(USART_TypeDef *USARTx, char cData)
{
	USART_SendData(USARTx, (uint8_t)cData);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)
	{
		;
	}
}


char USART_ReceiveByte(USART_TypeDef *USARTx)
{
	while((USART_GetFlagStatus(USARTx,USART_FLAG_RXNE)) == RESET)
	{
		;
	}
	return (uint8_t)USART_ReceiveData(USARTx);
}


void USART_SendStr(USART_TypeDef *USARTx, uint32_t ulLen, const char *pcData)
{
	for(uint32_t i=0; i<ulLen; i++)
	{
		USART_SendData(USARTx, (uint8_t)*(pcData++));
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)
		{
			;
		}
	}
}


#ifdef __IO_2USART

#include <stdio.h>
/*
 * 半主机模式就是通过仿真器实现开发板在电脑上的输入和输出
 * 这个接口在ARM内核中有实现，标准库默认使用了半主机模式。
 * MicroLib中内没有半主机模式，所以不需要禁用但是，据说MicroLib性能不好，所以使用标准库。
 * 
*/
//使用了该指令，如果仍然链接了使用半主机模式的函数，则链接器会报告错误。
#pragma import(__use_no_semihosting)

struct __FILE
{
	int handle; 
};

FILE __stdout;

//标准库中的_sys_exit使用了半主机模式，所以我们需要自己定义一个
void _sys_exit(int x)
{
	x = x;
}

//重定向c标准库的frintf函数和scanf函数
int fputc(int ch, FILE *f)
{
	ucUsart1_PutChar(ch, portMAX_DELAY);
	return (ch);
}

int fgetc(FILE *f)
{
	char ch;
	ucUsart1_GetChar(&ch, portMAX_DELAY);
	return ch;
}

#endif

