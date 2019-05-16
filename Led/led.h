#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#define __LED_OB_ON()			GPIO_ResetBits(GPIOC, GPIO_Pin_13)
#define __LED_OB_OFF()			GPIO_SetBits(GPIOC, GPIO_Pin_13)

#define __LED_EX_R_ON()			GPIO_ResetBits(GPIOA, GPIO_Pin_6)
#define __LED_EX_R_OFF()		GPIO_SetBits(GPIOA, GPIO_Pin_6)
#define __LED_EX_G_ON()			GPIO_ResetBits(GPIOA, GPIO_Pin_7)
#define __LED_EX_G_OFF()		GPIO_SetBits(GPIOA, GPIO_Pin_7)
#define __LED_EX_B_ON()			GPIO_ResetBits(GPIOB, GPIO_Pin_1)
#define __LED_EX_B_OFF()		GPIO_SetBits(GPIOB, GPIO_Pin_1)

extern uint8_t exucLED_Sta;
void vLedPWM_Init(void);

void vLed_Task(void *pvParameters);


#endif

