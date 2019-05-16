#include "read-line.h"
#include <string.h>


static char cPrompt[PROMPT_LEN] = {0};
static char cReadBuffer[READBUF_LEN] = {0};

static char ucEcho = 0x00;						/* char echo */
static const char cEraseSeq[] = "\b \b";		/* erase sequence */

#if 0
//需要底层提供输出和输入一个字符的函数例如：
#include <stdio.h>
static char prvGetChar(void)
{
	return getchar();
}

static void prvPutChar(char cChar)
{
	putchar(cChar);
}

static void prvPutString(const char * const pcChar)
{
	puts(pcChar);
}
#endif


//-------------------------------------------------------------
#include "usart.h"			//串口提供输入输出功能
#include "bsp_init.h"		//提供互斥量句柄和FreeRTOS的操作函数
static char prvGetChar(void)
{
	char rxtemp = 0;
	xSemaphoreTake(xUsart1_RxMutex, portMAX_DELAY);
	ucUsart1_GetChar(&rxtemp, portMAX_DELAY);
	xSemaphoreGive(xUsart1_RxMutex);
	return rxtemp;
}

static void prvPutChar(char cChar)
{
	xSemaphoreTake(xUsart1_TxMutex, portMAX_DELAY);
	ucUsart1_PutChar(cChar, portMAX_DELAY);
	xSemaphoreGive(xUsart1_TxMutex);
}

static void prvPutString(const char * const pcChar)
{
	xSemaphoreTake(xUsart1_TxMutex, portMAX_DELAY);
	ucUsart1_PutString(pcChar, strlen(pcChar), portMAX_DELAY);
	xSemaphoreGive(xUsart1_TxMutex);
}
//-------------------------------------------------------------


/**
* 函数说明： 从输入源中读取一行命令(必须以\n结束)，保存到Buffer中
* @param ucEcho:      1:字符回显，0:不回显
* @return normal:   获取的命令的字符串个数
*/
uint32_t ulReadLine(void)
{
	char *pcNow = cReadBuffer;		/* the point to ready receive. */
	char cChar = 0;
	uint32_t ulBufCnt = 0;			/* receive char cnt */
	uint32_t ulProLen = 0;			/* pcPrompt length */
	uint32_t ulOutColCnt = 0;		/* output column cnt */

	memset(cReadBuffer, 0, sizeof(cReadBuffer));
	/* print Prompt */
	if (*cPrompt)
	{
		ulProLen = strlen(cPrompt);
		ulOutColCnt = ulProLen;
		prvPutString(cPrompt);
	}
	for (;;)
	{
		cChar = prvGetChar();
		/**
		* Special character handling
		*/
		switch (cChar)
		{
		case 0x0D:			/*\r*/
			//break;
		case 0x0A:			/*\n*/
			if ((pcNow >= &cReadBuffer[0]) && \
				(pcNow <= &cReadBuffer[READBUF_LEN]))
			{
				*(pcNow+1) = '\0';
				if (ucEcho)
				{
					prvPutString("\r\n");
				}
				return (pcNow- cReadBuffer);
			}
			else
			{
				memset(cReadBuffer,0,sizeof(cReadBuffer));
				return 0;
			}

		case 0x03:						/* ^C - break */
			return 0;

		case 0x15:						/* ^U - erase line */
			while (ulOutColCnt > ulProLen)
			{
				if (ucEcho)
				{
					prvPutString(cEraseSeq);
				}
				ulOutColCnt--;
			}
			memset(cReadBuffer,0,sizeof(cReadBuffer));
			pcNow = cReadBuffer;
			ulBufCnt = 0x00;
			break;

		case 0x08:						/* ^H  - backspace	*/
		case 0x7F:						/* DEL - backspace	*/
			if( (ulOutColCnt > ulProLen) && (pcNow > &cReadBuffer[0]) && (ulBufCnt > 0x00) )
			{
				if(ucEcho)
				{
					prvPutString(cEraseSeq);
				}
				ulOutColCnt--;
				pcNow--;
				ulBufCnt--;
				*pcNow = '\0';			/* earse the receice char */
			}
			break;

		default:
			/**
			* Receive buffer is not full and must be a normal character
			* Increase the defense function, to prevent the pointer cross the border,
			* modify the memory
			*/
			if (ulBufCnt < sizeof(cReadBuffer) && \
			        (pcNow >= &cReadBuffer[0]) && \
			        (pcNow <= &cReadBuffer[READBUF_LEN]) && \
			        (cChar > 0x19 && cChar < 0x7F))
			{
				ulOutColCnt++;
				if(ucEcho)
				{
					/* echo input */
					prvPutChar(cChar);
				}
				*pcNow = cChar;
				pcNow++;
				ulBufCnt++;
			}
			else						/* Buffer full */
			{
				prvPutChar ('\a');
			}
		}
	}
}



uint8_t ucArgAnalyze(uint8_t *argc, char *argv[])
{
	uint32_t i = 0;

	while (*argc < MAX_ARGS)
	{
		/* skip any separator */
		while (cReadBuffer[i] == ' ')
		{
			i++;
		}
		/* end of line, no more args	*/
		if (cReadBuffer[i] == '\0')
		{
			argv[*argc] = NULL;
			return 0;
		}
		/* begin of argument string */
		argv[(*argc)++] = &cReadBuffer[i];
		/* find end of argument string */
		while (cReadBuffer[i] && cReadBuffer[i] != ' ')
		{
			i++;
		}
		/* end of line, no more args */
		if (cReadBuffer[i] == '\0')
		{
			argv[*argc] = NULL;
			return 0;
		}
		cReadBuffer[i++] = '\0';
	}
	return 1;
}


uint8_t ucChangePrompt(const char *cNewPro)
{
	if (cNewPro != NULL)
	{
		memset(cPrompt, 0, sizeof(cPrompt));
		strncpy(cPrompt, cNewPro, \
		        (sizeof(cPrompt)-1));
		return 0;
	}
	else
	{
		return 1;
	}
}

uint8_t ucChangeEcho(const uint8_t ucNewEcho)
{
	if (ucNewEcho == 0 || ucNewEcho == 1)
	{
		ucEcho = ucNewEcho;
		return 0;
	}
	else
	{
		return 1;
	}
}
