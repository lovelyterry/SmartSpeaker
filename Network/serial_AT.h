#ifndef __SERIAL_AT_H
#define __SERIAL_AT_H

#include "stm32f4xx.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"



#define Normal			(0x00)
#define ExecFail		(1<<0)
#define SendTimeOut		(1<<1)
#define ReceTimeOut		(1<<2)
#define ParaErr			(1<<3)
#define Overflow		(1<<4)


/* 定义AT接收缓冲区的大小 */
#define __AT_SIZE					(1024U)


const char *pcSerialAT_GetCachePoint(void);
uint8_t ucSerialAT_SendCmd(const char *pcCmd2Send, TickType_t xTicksToWait);
uint8_t ucSerialAT_WaitRes(const char **ppcSucBuf, const char **ppcErrBuf, TickType_t xTicksToWait);



#endif
