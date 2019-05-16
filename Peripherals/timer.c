#include "timer.h"


#define TIM2_COUNTER_CLOCK        42000000
#define TIM2_PRESCALER_VALUE      (SystemCoreClock/2/TIM2_COUNTER_CLOCK - 1)
#define TIM2_PERIOD_TIMING  	  (42 - 1)


void vTimer2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_DeInit(TIM2);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = TIM2_PERIOD_TIMING;
	TIM_TimeBaseStructure.TIM_Prescaler = TIM2_PRESCALER_VALUE;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM2, ENABLE);
}



void Delay_us(uint32_t uscnt)
{
	TIM_Cmd(TIM2, ENABLE);
	while(uscnt--)
	{
		while(TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) == RESET);
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	}
	TIM_Cmd(TIM2, DISABLE);
}

