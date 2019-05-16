#include "stm32f4xx.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#include "bsp_init.h"
#include "syslog.h"

#include "network.h"
#include "led.h"
#include "shell.h"
#include "audio.h"


//定义版本信息
static const char verision[] = "\r\nSmartSpeaker\r\nv0.20\r\n";


//定义任务堆栈的内存地址和大小
static const HeapRegion_t xHeapRegions[] =
{
	{(uint8_t *)0x10000000UL, 0x10000},		//CCM		64kByte
	{NULL, 0}
};


static void vFeedDog_Task(void *pvParameters);


int main(void)
{
	//设置中断向量表偏移
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000000);
	//必须选择为组4，中断均为抢占优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	//屏蔽中断，防止系统没有启动，而在中断里调用了系统的函数
	__set_PRIMASK(1);
	//创建RTOS对象（任务、队列、信号量等等）会隐含的调用pvPortMalloc()，
	//因此必须先注册任务堆栈
	vPortDefineHeapRegions(xHeapRegions);
	//初始化硬件
	ucBsp_Init();
	//创建任务
	xTaskCreate(vFeedDog_Task, \
				"FeedDog", \
				configMINIMAL_STACK_SIZE, \
				NULL, \
				5, \
				NULL);
	xTaskCreate(vLed_Task, \
				"Led", \
				configMINIMAL_STACK_SIZE, \
				NULL, \
				2, \
				NULL);
	xTaskCreate(vShell_Task, \
				"Shell", \
				(configMINIMAL_STACK_SIZE*2), \
				NULL, \
				2, \
				NULL);
	xTaskCreate(vNet_Task, \
				"Net", \
				(configMINIMAL_STACK_SIZE*8), \
				NULL, \
				3, \
				NULL);
	xTaskCreate(vAudio_Task, \
				"Audio", \
				(configMINIMAL_STACK_SIZE), \
				NULL, \
				3, \
				NULL);
	ts_printf("\r\n%scompile at %s %s\r\n\r\n", verision, __DATE__, __TIME__);
	//启动调度系统
	vTaskStartScheduler();
	USART_SendStr(USART1, sizeof("\r\n#[error_msg]: Scheduler Over!\r\n") - 1, "\r\n#[error_msg]: Scheduler Over!\r\n");
	__set_PRIMASK(1);
	for(;;)
	{
		;
	}
}


//----------------------------------------------------------------------
#include "wdog.h"

static void vFeedDog_Task(void *pvParameters)
{
	static portTickType xLastWakeTime;
	
	vIWdg_Init(3000);
	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		IWDG_Feed();
		vTaskDelayUntil(&xLastWakeTime, 1000/portTICK_RATE_MS);
	}
}
//----------------------------------------------------------------------


#if 0
//用于测试FPU是否启动
void vFloat_Delay(void)
{
	float x = 50.0f;
	
	while (x > 0.0001f)
	{
		x = x/1.0001f;
	}
}
#endif


//----------------------------------------------------------------------
//FreeRTOS 钩子函数
void vApplicationMallocFailedHook(void)
{
	log_record("\r\n#[error_msg]: Malloc failed!\r\n");
	__set_PRIMASK(1);
	for (;;)
	{
		;
	}
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	log_record("\r\n#[error_msg]: Stack overflow! Handle: %x, Task: %s\r\n", pxTask, pcTaskName);
	__set_PRIMASK(1);
	for (;;)
	{
		;
	}
}
//----------------------------------------------------------------------


#ifdef USE_FULL_ASSERT
/**
* @brief Reports the name of the source file and the source line number
* where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	log_record("\r\n#[error_msg]: Wrong parameters value: file %s on line %d\r\n", file, line);
	__set_PRIMASK(1);
	for (;;)
	{
		;
	}
}
#endif
