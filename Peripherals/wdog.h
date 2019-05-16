#ifndef __WDOG_H
#define	__WDOG_H


#include "stm32f4xx.h"



void vIWdg_Init(uint32_t ulIWDGTime);
void IWDG_Feed(void);
void vWWdg_Init(uint8_t ucTreg, uint8_t ucWreg, uint32_t WWDG_Prescaler);





#endif
