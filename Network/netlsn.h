#ifndef __NETLSN_H
#define __NETLSN_H


#include <stdint.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


//如果定义了这个宏，那么监听收到的文件就保存在文件系统中
//否则保存在内存的公共缓冲区中
//#define __NETLSN_USE_FS

#if defined(__NETLSN_USE_FS)

//需要文件系统支持，提供文件缓存功能
#include "ff.h"
#define LSN_FILE		"0:/cherryFly/netlsn.txt"

#else

#include "bsp_init.h"

#define LSN_BUFFER		PubBuf
#define LSN_MAXSIZE		PUBBUF_SIZE

#endif



//监听完毕后会发送这个信号量
extern SemaphoreHandle_t xNetLsn_RecBinary;
//发送这个信号量会强制终止监听
extern SemaphoreHandle_t xNetLsn_CloBinary;
//监听资源互斥量，因为8266是唯一的资源，所以只允许一个任务在监听
extern SemaphoreHandle_t xNetLsn_Mutex;


void vNetLsn_Init(void);
void vNetLsn_Close(void);
void vNetLsn_Task(void *pvParameters);


static inline void vNet_LsnStart(uint32_t ulMaxWait)
{
	xSemaphoreTake(xNetLsn_Mutex, portMAX_DELAY);
	xTaskCreate(vNetLsn_Task, \
				"NetLsn", \
				(configMINIMAL_STACK_SIZE*2), \
				(void *)&ulMaxWait, \
				5, \
				NULL);
	xSemaphoreGive(xNetLsn_Mutex);
}




#endif
