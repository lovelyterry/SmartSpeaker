#include "serial_AT.h"
#include "usart.h"
#include <string.h>


static char pcSerialAT_Cache[__AT_SIZE] = {0x00};


const char *pcSerialAT_GetCachePoint(void)
{
	return (const char *)pcSerialAT_Cache;
}


//pcCmd2Send：	需要发送的命令字符串指针
uint8_t ucSerialAT_SendCmd(const char *pcCmd2Send, TickType_t xTicksToWait)
{
	uint8_t res = Normal;

	if (pcCmd2Send == NULL)
	{
		res |= ParaErr;
	}
	else
	{
		xSemaphoreTake(xUsart2_TxMutex, portMAX_DELAY);
		if (ucUsart2_PutString(pcCmd2Send, strlen(pcCmd2Send), xTicksToWait) == 1)
		{
			//发送失败的话清空发送队列，防止对下次发送造成影响
			xQueueReset(xUsart2_TxQueue);
			res |= SendTimeOut;
		}
		else
		{
			//发送成功的话清空接收队列，准备接受数据
			xQueueReset(xUsart2_RxQueue);
			res |= Normal;
		}
		xSemaphoreGive(xUsart2_TxMutex);
	}
	return res;
}


//ppcSucBuf:	只有收到这个字符串数组里的所有字符串，才判定执行成功。
//ppcErrBuf:	只要收到这个字符串数组里的一个字符串，就判定执行失败。
uint8_t ucSerialAT_WaitRes(const char **ppcSucBuf, const char **ppcErrBuf, TickType_t xTicksToWait)
{
	char rxtemp = 0;
	uint8_t res = Normal;

	if (ppcSucBuf == NULL || ppcErrBuf == NULL)
	{
		res |= ParaErr;
	}
	else
	{
		xSemaphoreTake(xUsart2_RxMutex, portMAX_DELAY);
		memset(pcSerialAT_Cache, 0, sizeof (pcSerialAT_Cache));
		for (uint16_t i=0; i<__AT_SIZE; i++)
		{
			if (ucUsart2_GetChar(&rxtemp, xTicksToWait) == 0)
			{
				uint8_t ucCmd_NFound = 0;
				pcSerialAT_Cache[i] = rxtemp;
				for (uint8_t j=0; ppcSucBuf[j]!=NULL; j++)
				{
					if (strstr(pcSerialAT_Cache, ppcSucBuf[j]) == NULL)
					{
						ucCmd_NFound++;
						break;
					}
				}
				if (ucCmd_NFound == 0)
				{
					res |= Normal;
					xSemaphoreGive(xUsart2_RxMutex);
					return res;
				}
				for (uint8_t j=0; ppcErrBuf[j]!=NULL; j++)
				{
					if (strstr(pcSerialAT_Cache, ppcErrBuf[j]) != NULL)
					{
						res |= ExecFail;
						xSemaphoreGive(xUsart2_RxMutex);
						return res;
					}
				}
			}
			else
			{
				res |= ReceTimeOut;
				xSemaphoreGive(xUsart2_RxMutex);
				return res;
			}
		}
		res |= Overflow;
		xSemaphoreGive(xUsart2_RxMutex);
		return res;
	}
	return res;
}


