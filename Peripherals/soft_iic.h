#ifndef __SOFT_IIC_H
#define __SOFT_IIC_H

//-- Includes -----------------------------------------------------------------
#include "stm32f4xx.h"

//-- Enumerations -------------------------------------------------------------
// I2C acknowledge
typedef enum{
  ACK  = 0,
  NACK = 1,
}eIIC_Ack;

// I2C Error codes
typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
}eIIC_Error;

#define IIC_WR	0
#define IIC_RD	1

//=============================================================================
// Initializes the ports for I2C interface.
//=============================================================================
void vSoftIIC_Init(void);

//=============================================================================
// Writes a start condition on I2C-Bus.
//       _____
// SDA:       |_____
//       _______
// SCL:         |___
//=============================================================================
void vSoftIIC_Start(void);
//=============================================================================
// Writes a stop condition on I2C-Bus.
//              _____
// SDA:   _____|
//            _______
// SCL:   ___|
//=============================================================================
void vSoftIIC_Stop(void);

eIIC_Error xSoftIIC_WriteByte(const uint8_t uctxByte);
eIIC_Error xSoftIIC_ReadByte(uint8_t *pucrxByte, eIIC_Ack ack);

#endif
