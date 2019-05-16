#include "wdog.h"



volatile uint32_t LsiFreq = 0;
volatile uint32_t PeriodValue = 0;
volatile uint32_t CaptureNumber = 0;


/*
*********************************************************************************************************
*	函 数 名: bsp_InitIwdg
*	功能说明: 独立看门狗时间配置函数
*	形    参：IWDGTime: 0 ---- 0x0FFF
*			  独立看门狗时间设置,单位为ms,IWDGTime = 1000 大约就是一秒的
*             时间，这里没有结合TIM5测得实际LSI频率，只是为了操作方便取了
*             一个估计值超过IWDGTime不进行喂狗的话系统将会复位。
*			  LSI = 34000左右
*	返 回 值: 无		        
*********************************************************************************************************
*/
void vIWdg_Init(uint32_t ulIWDGTime)
{
	/* 检测系统是否从独立看门狗复位中恢复 */
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{		
		/* 清除复位标志 */
		RCC_ClearFlag();
	}
	else
	{
		/* 标志没有设置 */
	}
	/* 使能LSI */
	RCC_LSICmd(ENABLE);
	/* 等待直到LSI就绪 */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{}
	LsiFreq = 32000;
	
	/* 写入0x5555表示允许访问IWDG_PR 和IWDG_RLR寄存器 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	/*  LSI/32 分频*/
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	
	/*特别注意，由于这里ulIWDGTime的最小单位是ms, 所以这里重装计数的
	  计数时 需要除以1000
	 Counter Reload Value = (ulIWDGTime / 1000) /(1 / IWDG counter clock period)
	                      = (ulIWDGTime / 1000) / (32/LSI)
	                      = (ulIWDGTime / 1000) / (32/LsiFreq)
	                      = LsiFreq * ulIWDGTime / 32000
	 实际测试LsiFreq = 34000，所以这里取1的时候 大概就是1ms 
	*/
	IWDG_SetReload(ulIWDGTime);
	
	/* 重载IWDG计数 */
	IWDG_ReloadCounter();
	
	/* 使能 IWDG (LSI oscillator 由硬件使能) */
	IWDG_Enable();		
}

/*
*********************************************************************************************************
*	函 数 名: IWDG_Feed
*	功能说明: 喂狗函数
*	形    参：无
*	返 回 值: 无		        
*********************************************************************************************************
*/
void IWDG_Feed(void)
{
	IWDG_ReloadCounter();
}

/*
*********************************************************************************************************
*	函 数 名: vWWdg_Init
*	功能说明: 窗口看门狗配置 
*	形    参：
*             ucTreg       : T[6:0],计数器值 	范围0x40 到 0x7F                                               
*             ucWreg       : W[6:0],窗口值     必须小于 0x80
*            WWDG_Prescaler : 窗口看门狗分频	PCLK1 = 42MHz
*                             WWDG_Prescaler_1: WWDG counter clock = (PCLK1/4096)/1
*							  WWDG_Prescaler_2: WWDG counter clock = (PCLK1/4096)/2
*							  WWDG_Prescaler_4: WWDG counter clock = (PCLK1/4096)/4
*							  WWDG_Prescaler_8: WWDG counter clock = (PCLK1/4096)/8 
*	返 回 值: 无		        
*********************************************************************************************************
*/
void vWWdg_Init(uint8_t ucTreg, uint8_t ucWreg, uint32_t WWDG_Prescaler)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* 检测系统是否从窗口看门狗复位中恢复 */
	if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
	{ 	
		/* 清除复位标志 */
		RCC_ClearFlag();
	}
	else
	{
		/* WWDGRST 标志没有设置 */
	}
	/* 使能WWDG时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
	/* 
	   窗口看门狗分频设置：
	   比如选择WWDG_Prescaler_8
	   (PCLK1 (42MHz)/4096)/8 = 1281 Hz (~780 us)  
	*/
	WWDG_SetPrescaler(WWDG_Prescaler);
	/* 
	 设置窗口值是ucWreg，用户必须在小于ucWreg且大于0x40时刷新计数
	 器，要不会造成系统复位。
	*/
	WWDG_SetWindowValue(ucWreg);
	/* 
	 使能WWDG，设置计数器
	 比如设置ucTreg=127 8分频时，那么溢出时间就是= ~780 us * 64 = 49.92 ms 
	 窗口看门狗的刷新时间段是: ~780 * (127-80) = 36.6ms < 刷新窗口看门狗 < ~780 * 64 = 49.9ms
	*/
	WWDG_Enable(ucTreg);
	/* 清除EWI中断标志 */
	WWDG_ClearFlag();	
	/* 使能EW中断 */
	WWDG_EnableIT();
	
	/* 设置 WWDG 的NVIC */
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}




void WWDG_IRQHandler(void)
{
	
}
