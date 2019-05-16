#include "soft_iic.h"


#define IIC_PORT		GPIOB
#define RCC_IIC_PORT	RCC_AHB1Periph_GPIOB
#define IIC_SDA_PIN		GPIO_Pin_7
#define IIC_SCL_PIN		GPIO_Pin_6


#define SDA_HIGH()		(IIC_PORT->BSRRL = IIC_SDA_PIN)
#define SDA_LOW()		(IIC_PORT->BSRRH = IIC_SDA_PIN)
#define SDA_READ()		((IIC_PORT->IDR & IIC_SDA_PIN) != 0x00)

#define SCL_HIGH()		(IIC_PORT->BSRRL = IIC_SCL_PIN)
#define SCL_LOW()		(IIC_PORT->BSRRH = IIC_SCL_PIN)
#define SCL_READ()		((IIC_PORT->IDR & IIC_SCL_PIN) != 0x00)

static void prvIIC_Delay(uint32_t ulnbrOft);


void vSoftIIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_IIC_PORT, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN | IIC_SDA_PIN;
	GPIO_Init(IIC_PORT, &GPIO_InitStructure);
	
	SDA_HIGH();
	SCL_HIGH();
	vSoftIIC_Stop();
	vSoftIIC_Stop();
}


void vSoftIIC_Start(void)
{
	SDA_HIGH();
	prvIIC_Delay(5);
	SCL_HIGH();
	prvIIC_Delay(5);
	SDA_LOW();
	prvIIC_Delay(10);
	SCL_LOW();
	prvIIC_Delay(10);
}

void vSoftIIC_Stop(void)
{
	SCL_LOW();
	prvIIC_Delay(5);
	SDA_LOW();
	prvIIC_Delay(5);
	SCL_HIGH();
	prvIIC_Delay(10);
	SDA_HIGH();
	prvIIC_Delay(10);
}

eIIC_Error xSoftIIC_WriteByte(const uint8_t uctxByte)
{
	eIIC_Error error = NO_ERROR;
	uint8_t mask;
	
	for (mask = 0x80; mask > 0; mask >>= 1)
	{
		if ((mask & uctxByte) == 0)
		{
			SDA_LOW();
		}
		else
		{
			SDA_HIGH();
		}
		SCL_HIGH();
		prvIIC_Delay(5);
		SCL_LOW();
		prvIIC_Delay(5);
	}
	SDA_HIGH();
	SCL_HIGH();
	prvIIC_Delay(5);
	if (SDA_READ()) 
	{
		error = ACK_ERROR;
	}
	SCL_LOW();
	prvIIC_Delay(10);
	return error;
}

eIIC_Error xSoftIIC_ReadByte(uint8_t *pucrxByte, eIIC_Ack ack)
{
	eIIC_Error error = NO_ERROR;
	uint8_t mask;
	*pucrxByte = 0x00;
	
	SDA_HIGH();
	for (mask = 0x80; mask > 0; mask >>= 1)
	{ 
		SCL_HIGH();
		prvIIC_Delay(5);
		if (SDA_READ())
		{
			*pucrxByte |= mask;
		}
		SCL_LOW();
		prvIIC_Delay(5);
	}
	if (ack == ACK)
	{
		SDA_LOW();
	}
	else
	{
		SDA_HIGH();
	}
	SCL_HIGH();
	prvIIC_Delay(5);
	SCL_LOW();
	prvIIC_Delay(5);
	SDA_HIGH();
	prvIIC_Delay(10);
	return error;
}

static  void prvIIC_Delay(uint32_t ulnbrOft)
{
	volatile uint32_t i;
	
	for (i = 0; i < ulnbrOft*10; i++)
	{
		i = i;
	}
}
