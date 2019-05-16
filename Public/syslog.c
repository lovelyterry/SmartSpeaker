#include "syslog.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "usart.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"



static char logbuf[1024] = {0};

int log_record(const char *format, ...)
{
	va_list args;

	memset(logbuf,0,sizeof (logbuf));
	va_start(args, format);
	int result = vsprintf(logbuf,format, args);
	va_end(args);
	
	USART_SendStr(USART1, strlen(logbuf), logbuf);
	
	return result;
}


int ts_printf(const char *format, ...)
{
	va_list args;
	
	xSemaphoreTake(xUsart1_TxMutex, portMAX_DELAY);
	va_start(args, format);
	int result = vprintf(format, args);
	va_end(args);
	xSemaphoreGive(xUsart1_TxMutex);
	return result;
}

