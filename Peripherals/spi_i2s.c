#include "spi_i2s.h"


SemaphoreHandle_t xI2s2DMA_TxBinary = NULL;
SemaphoreHandle_t xI2s2DMA_RxBinary = NULL;



void vI2s2_GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2); 		//PB12,AF5  I2S_LRCK
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);		//PB13,AF5  I2S_SCLK 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);			//PC3 ,AF5  I2S_DACDATA 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_SPI2);			//PC6 ,AF5  I2S_MCK
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_I2S2ext);		//PC2 ,AF6  I2S_ADCDATA
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void vI2s2_ClockInit(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	/*
		I2S有专门的时钟源
	*/
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	RCC_PLLI2SCmd(ENABLE);
	while (!RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY))
	{
		;
	}
}



void vI2s2_ModeInit(uint16_t I2S_Standard, \
				uint16_t I2S_Mode, \
				uint16_t I2S_CPOL, \
				uint16_t I2S_DataFormat, \
				uint32_t I2S_AudioFreq\
				)
{
	I2S_InitTypeDef I2S_InitStructure;
	
	I2S_InitStructure.I2S_Mode = I2S_Mode;
	I2S_InitStructure.I2S_Standard = I2S_Standard;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL;
	
	I2S_Init(SPI2, &I2S_InitStructure);
	I2S_FullDuplexConfig(I2S2ext, &I2S_InitStructure);
	I2S_Cmd(SPI2, ENABLE);
	I2S_Cmd(I2S2ext, ENABLE);
}



void vI2s2_BinaryInit(void)
{
	static uint8_t I2s2_BinarySetFlag = 0x00;
	
	if (I2s2_BinarySetFlag == 0x00)
	{
		vSemaphoreCreateBinary(xI2s2DMA_TxBinary);
		vSemaphoreCreateBinary(xI2s2DMA_RxBinary);
		if ((xI2s2DMA_TxBinary && xI2s2DMA_RxBinary) == NULL)
		{
			for (;;)
			{
				;
			}
		}
		I2s2_BinarySetFlag = 0x01;
	}
}


void vI2s2DMA_TxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(I2S2_TX_STREAM);
	while (DMA_GetCmdStatus(I2S2_TX_STREAM) != DISABLE)
	{
		;
	}
	DMA_ClearITPendingBit(I2S2_TX_STREAM, DMA_IT_FEIF4 | DMA_IT_DMEIF4 \
						| DMA_IT_TEIF4 | DMA_IT_HTIF4 | DMA_IT_TCIF4);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)pucBufA;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = usNum;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(I2S2_TX_STREAM, &DMA_InitStructure);

	DMA_DoubleBufferModeConfig(I2S2_TX_STREAM, (uint32_t)pucBufB, DMA_Memory_0);
	DMA_DoubleBufferModeCmd(I2S2_TX_STREAM, ENABLE);

	DMA_ITConfig(I2S2_TX_STREAM, DMA_IT_TC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0A;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void vI2s2DMA_RxInit(uint8_t* pucBufA, uint8_t *pucBufB, uint16_t usNum)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(I2S2_RX_STREAM);
	while (DMA_GetCmdStatus(I2S2_RX_STREAM) != DISABLE)
	{
		;
	}
	DMA_ClearITPendingBit(I2S2_RX_STREAM, DMA_IT_FEIF3 | DMA_IT_DMEIF3 \
						| DMA_IT_TEIF3 | DMA_IT_HTIF3 | DMA_IT_TCIF3);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2S2ext->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)pucBufA;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = usNum;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(I2S2_RX_STREAM, &DMA_InitStructure);

	DMA_DoubleBufferModeConfig(I2S2_RX_STREAM, (uint32_t)pucBufB, DMA_Memory_0);
	DMA_DoubleBufferModeCmd(I2S2_RX_STREAM, ENABLE);
	
	DMA_ITConfig(I2S2_RX_STREAM, DMA_IT_TC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0x0A;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void vI2s2DMA_TxStart(void)
{
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	DMA_Cmd(I2S2_TX_STREAM, ENABLE);
}

void vI2s2DMA_RxStart(void)
{
	SPI_I2S_DMACmd(I2S2ext, SPI_I2S_DMAReq_Rx, ENABLE);
	DMA_Cmd(I2S2_RX_STREAM, ENABLE);
}

void vI2s2DMA_TxStop(void)
{
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, DISABLE);
	DMA_Cmd(I2S2_TX_STREAM, DISABLE);
}

void vI2s2DMA_RxStop(void)
{
	SPI_I2S_DMACmd(I2S2ext, SPI_I2S_DMAReq_Rx, DISABLE);
	DMA_Cmd(I2S2_RX_STREAM, DISABLE);
}



void DMA1_Stream4_IRQHandler(void)
{
	BaseType_t HigherPriorityTaskWoken = pdTRUE;
	
	if(DMA_GetITStatus(I2S2_TX_STREAM, DMA_IT_TCIF4) == SET)
	{
		DMA_ClearITPendingBit(I2S2_TX_STREAM, DMA_IT_TCIF4);
		xSemaphoreGiveFromISR(xI2s2DMA_TxBinary, &HigherPriorityTaskWoken);
	}
}


void DMA1_Stream3_IRQHandler(void)
{
	BaseType_t HigherPriorityTaskWoken = pdTRUE;
	
	if(DMA_GetITStatus(I2S2_RX_STREAM, DMA_IT_TCIF3) == SET)
	{
		DMA_ClearITPendingBit(I2S2_RX_STREAM, DMA_IT_TCIF3);
		xSemaphoreGiveFromISR(xI2s2DMA_RxBinary, &HigherPriorityTaskWoken);
	}
}


//--------------------------------------------------------------------------------------------

void SPI1_Init(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL  = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
	
	SPI1_ReadWriteByte(0xff);//启动传输
}

uint8_t SPI1_ReadWriteByte(uint8_t TxData)
{
	//检查指定的SPI标志位设置与否:发送缓存空标志位
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
	{
		; 
	}
	//通过外设SPIx发送一个数据
	SPI_I2S_SendData(SPI1, TxData); 
	//检查指定的SPI标志位设置与否:接受缓存非空标志位
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
	{
		;
	}
	//返回通过SPIx最近接收的数据
	return SPI_I2S_ReceiveData(SPI1);
}




















