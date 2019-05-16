#ifndef __BSP_INIT_H
#define __BSP_INIT_H

#include "stm32f4xx.h"

//公共缓冲区大小
#define PUBBUF_SIZE		(12*1204U)

extern uint8_t PubBuf[PUBBUF_SIZE+2];



uint8_t ucBsp_Init(void);







#endif
