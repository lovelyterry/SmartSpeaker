#include "netlsn.h"
//需要用到串口的信号量和互斥量
#include "usart.h"
//需要用到网络接收缓冲区
#include "network.h"
#include <string.h>


//监听完毕后会发送这个信号量
SemaphoreHandle_t xNetLsn_RecBinary = NULL;
//发送这个信号量会强制终止监听
SemaphoreHandle_t xNetLsn_CloBinary = NULL;
//监听资源互斥量，因为8266是唯一的资源，所以只允许一个任务在监听
SemaphoreHandle_t xNetLsn_Mutex = NULL;



void vNetLsn_Init(void)
{
	static uint8_t xNetLsn_OSSetFlag = 0;//信号量建立标志
	
	if (xNetLsn_OSSetFlag == 0)
	{
		vSemaphoreCreateBinary(xNetLsn_RecBinary);
		vSemaphoreCreateBinary(xNetLsn_CloBinary);
		xNetLsn_Mutex = xSemaphoreCreateMutex();
		xNetLsn_OSSetFlag = 0x01;
	}
}


void vNetLsn_Close(void)
{
	xSemaphoreGive(xNetLsn_CloBinary);
}

#if defined(__NETLSN_USE_FS)
FIL xlsn_fp = {0};			//缓存文件指针(这里不使用动态分配)
#endif


//8266的串口监听任务，用DMA接收8266发送给32的数据，并保存到文件中或转存到缓冲区
//参数为监听所保存的文件，如果__NETLSN_USE_FS未被定义，则该参数无效
void vNetLsn_Task(void *pvParameters)
{
	uint8_t ucErrCnt = 0;		//超时次数统计
#if defined(__NETLSN_USE_FS)
	uint32_t bw = 0;			//用于指示写入了多少Byte

	//打开数据缓存文件
	f_open(&xlsn_fp, LSN_FILE , FA_CREATE_ALWAYS | FA_WRITE);
#else
	uint32_t ulRecCnt = 0;		//用于统计接收了多少包数据
#endif
	//获取串口2接收互斥量
	xSemaphoreTake(xUsart2_RxMutex, portMAX_DELAY);
	//清空DMA双缓存
	memset(pNET_RECEIVE_BUF0, 0, NET_BUFFER_SIZE);
	memset(pNET_RECEIVE_BUF1, 0, NET_BUFFER_SIZE);
	//设置串口接收DMA双缓存
	vUsart2DMA_RxInit(pNET_RECEIVE_BUF0, pNET_RECEIVE_BUF1, NET_BUFFER_SIZE);
	//开启接收数据
	vUsart2DMA_RxStart();
	
	//清空串口2接收相关的信号量，注意需要在开启DMA后再清空
	xSemaphoreTake(xUsart2_IDLE_Binary, 0);
	xSemaphoreTake(xUsart2_DMA_RxBinary, 0);
	//清空强制关闭监听的信号量
	xSemaphoreTake(xNetLsn_CloBinary, 0);
	//清空监听完成的信号量
	xSemaphoreTake(xNetLsn_RecBinary,0);
	
	for (;;)
	{
		//等待DMA中断发生,每隔10毫秒查看总线空闲中断是否发生
		if (xSemaphoreTake(xUsart2_DMA_RxBinary, 10/portTICK_RATE_MS) == pdTRUE)
		{
			ucErrCnt = 0;	//超时统计清零
#if defined(__NETLSN_USE_FS)	//写入文件系统
			f_write(&xlsn_fp, \
					DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF0) : (char *)(pNET_RECEIVE_BUF1) , \
					NET_BUFFER_SIZE, \
					&bw);
					
			memset(DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF0) : (char *)(pNET_RECEIVE_BUF1), \
					0, \
					NET_BUFFER_SIZE);
#else						//写入公共缓冲区
			ulRecCnt++;		//数据包计数器加一
			if (ulRecCnt*NET_BUFFER_SIZE < LSN_MAXSIZE)
			{
				memset((LSN_BUFFER + (ulRecCnt-1)*NET_BUFFER_SIZE), \
						0, \
						NET_BUFFER_SIZE);
				memcpy((LSN_BUFFER + (ulRecCnt-1)*NET_BUFFER_SIZE), \
						DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF0) : (char *)(pNET_RECEIVE_BUF1), \
						NET_BUFFER_SIZE);
				memset(DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF0) : (char *)(pNET_RECEIVE_BUF1), \
						0, \
						NET_BUFFER_SIZE);
			}
			else
			{
				//缓冲区溢出，停止接收
				break;
			}
#endif /* defined(__NETLSN_USE_FS) */
		}
		else
		{
			//发生了总线空闲中断，说明数据接收已经结束了
			//将最后一个组数据写入缓存文件中
			if (xSemaphoreTake(xUsart2_IDLE_Binary, 0) == pdTRUE)
			{
				uint32_t ulRecByte = 0;//用于统计最后一包数据多少字节
				
				ulRecByte = NET_BUFFER_SIZE - DMA_GetCurrDataCounter(USART2_RX_STREAM);
#if defined(__NETLSN_USE_FS)
				f_write(&xlsn_fp, \
						DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF1) : (char *)(pNET_RECEIVE_BUF0) , \
						ulRecByte, \
						&bw);
				memset(DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF1) : (char *)(pNET_RECEIVE_BUF0) , \
						0, \
						NET_BUFFER_SIZE);
#else
				ulRecCnt++;		//数据包计数器加一
				if ((ulRecCnt-1)*NET_BUFFER_SIZE+ulRecByte < LSN_MAXSIZE)
				{
					memset((LSN_BUFFER + (ulRecCnt-1)*NET_BUFFER_SIZE), \
							0, \
							NET_BUFFER_SIZE + 0x01);
					memcpy((LSN_BUFFER + (ulRecCnt-1)*NET_BUFFER_SIZE), \
							DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF1) : (char *)(pNET_RECEIVE_BUF0) , \
							ulRecByte);
					memset(DMA_GetCurrentMemoryTarget(USART2_RX_STREAM) ? (char *)(pNET_RECEIVE_BUF1) : (char *)(pNET_RECEIVE_BUF0) , \
							0, \
							NET_BUFFER_SIZE);
				}
				else
				{
					//缓冲区溢出，停止接收
					break;
				}
#endif
				//结束监听
				break;
			}
			else
			{
				//判断是否超时
				if (++ucErrCnt > (*(uint32_t *)pvParameters)/10)
				{
					break;
				}
				//如果收到强制关闭监听的信号
				if (xSemaphoreTake(xNetLsn_CloBinary, 0) == pdTRUE)
				{
					break;
				}
			}
		}
	}
	vUsart2DMA_RxStop();
#if defined(__NETLSN_USE_FS)
	f_close(&xlsn_fp);
#endif
	//释放串口2互斥量
	xSemaphoreGive(xUsart2_RxMutex);
	//发送接收完毕信号量
	xSemaphoreGive(xNetLsn_RecBinary);
	//删除任务
	vTaskDelete(NULL);
}


