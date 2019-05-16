#include "esp8266.h"
#include "serial_AT.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/*
 *ESP8266相关的引脚设置
 *CHPD: 高电平使能
 *REST: 低电平复位
*/
#define ESP_PORT		GPIOA
#define ESP_REST		GPIO_Pin_1
#define ESP_CHPD		GPIO_Pin_1
#define RCC_ESP_PORT	RCC_AHB1Periph_GPIOA

const char *ppcESP_SuccRes[] = {"OK\r\n", NULL};
const char *ppcESP_FailRes[] = {"FAIL\r\n", "ERROR\r\n", NULL};




void vEsp_Init(void)
{
	vEsp_Reset();
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_ESP_PORT, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = ESP_REST | ESP_CHPD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(ESP_PORT, &GPIO_InitStructure);
	//关闭串口接收中断，防止8266初始化打印的数据对系统造成污染
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	GPIO_SetBits(ESP_PORT, ESP_CHPD | ESP_REST);
	
	/*从8266上电开始到能开始接受AT指令至少需等待200毫秒*/
	vTaskDelay(250/portTICK_RATE_MS);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#ifndef __TAPPING_MODE
	//如果不窃听的话，关闭回显,可以有效节省字符查找所需的时间
	if (ucEsp_ATE(0) != 0x00)
	{
		vTaskDelay(50/portTICK_RATE_MS);
		vEsp_Reset();
		vEsp_Init();
	}
#endif /* __TAPPING_MODE */
	//波特率再高上去就会出问题了，应该是8266的问题
	if (ucEsp_DataExSpeedCfg(921600))
	{
		vTaskDelay(50/portTICK_RATE_MS);
		vEsp_Reset();
		vEsp_Init();
	}
	//测试初始化是否成功
	if (ucEsp_AT() != 0x00)
	{
		vTaskDelay(50/portTICK_RATE_MS);
		vEsp_Reset();
		vUsart2_Init(115200);
		vEsp_Init();
	}
	vTaskDelay(50/portTICK_RATE_MS);
}

void vEsp_Reset(void)
{
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	GPIO_ResetBits(ESP_PORT, ESP_REST);
	vTaskDelay(100/portTICK_RATE_MS);
	GPIO_SetBits(ESP_PORT, ESP_REST);
	vTaskDelay(250/portTICK_RATE_MS);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void vEsp_LowPower(const uint8_t ucMode)
{
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	if (ucMode)
	{
		GPIO_ResetBits(ESP_PORT, ESP_CHPD);
	}
	else
	{
		GPIO_SetBits(ESP_PORT, ESP_CHPD);
	}
	vTaskDelay(250/portTICK_RATE_MS);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

//对单纯的调整8266的波特率进行了封装，如果设置8266的波特率成功，那么也修改32的波特率
uint8_t ucEsp_DataExSpeedCfg(uint32_t ulSpeed)
{
	uint8_t res;
	
	res = ucEsp_UART_CUR(ulSpeed);
	if (res == 0x00)
	{
		vUsart2_Init(ulSpeed);
	}
	/*需等待一段时间让8266反应过来*/
	vTaskDelay(100/portTICK_RATE_MS);
	return res;
}

//用于调试8266,可以通过命令行直接把指令发送给8266
//返回的结果请开启窃听模式查看
uint8_t ucEsp_Debug(const char *pcSend)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];

	if (pcSend != NULL)
	{
		snprintf(pcCmd, sizeof(pcCmd)-1, "%s\r\n", pcSend);
		ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	}
	else
	{
		ucExecRes = 0xFF;
	}
	return ucExecRes;
}



uint8_t ucEsp_AT(void)
{
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}



uint8_t ucEsp_ATE(const uint8_t ucEcho)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "ATE%u\r\n", ucEcho);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}



uint8_t ucEsp_RESTORE(void)
{
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT+RESTORE\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}



/*
const uint8_t ucMode:
1：Station 模式
2：SoftAP 模式
3：SoftAP+Station 模式
*/
uint8_t ucEsp_CWMODE_CUR(const uint8_t ucMode)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CWMODE_CUR=%u\r\n", ucMode);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}



uint8_t ucEsp_UART_CUR(const uint32_t ulBaudRate)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+UART_CUR=%d,8,1,0,0\r\n", ulBaudRate);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}


/*
• <ssid>：字符串参数，接入点名称
• <pwd>：字符串参数，密码长度范围：8 ~ 64 字节
ASCII
• <chl>：通道号
• <ecn>：加密方式，不支持 WEP
 0：OPEN
 2：WPA_PSK
 3：WPA2_PSK
 4：WPA_WPA2_PSK
• [<max conn>]（选填参数）：允许连⼊入 ESP8266
SoftAP 的最多 Station 数⽬目，取值范围 [1, 4]。
• [<ssid hidden>]（选填参数）：默认为 0，开启
广播 ESP8266 SoftAP SSID。
 0：广播 SSID
 1：不广播 SSID
*/
uint8_t ucEsp_CWSAP_CUR(const char *pSSID, const char *pPassWord, const uint8_t ucChl, const uint8_t ucEcn)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CWSAP_CUR=\"%s\",\"%s\",%u,%u\r\n", pSSID, pPassWord, ucChl, ucEcn);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}


uint8_t ucEsp_CWJAP_CUR(const char *pSSID, const char *pPassWord)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", pSSID, pPassWord);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 16*1000/portTICK_RATE_MS);
	return ucExecRes;
}


uint8_t ucEsp_CWJAP(void)
{
	const char *ppcESP_SuccRes[] = {"+CWJAP:", "OK", NULL};
	
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT+CWJAP?\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 0x01*1000/portTICK_RATE_MS);
	return ucExecRes;
}



uint8_t ucEsp_CWLAP(void)
{
	const char *ppcESP_SuccRes[] = {"OK", NULL};
	
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT+CWLAP\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 0x01*1000/portTICK_RATE_MS);
	return ucExecRes;
}

uint8_t ucEsp_CIPSTART(const char *pcProtocol, const char* pcIpAddr, const uint16_t usPort)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CIPSTART=\"%s\",\"%s\",%d\r\n", pcProtocol, pcIpAddr, usPort);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 0x0F*1000/portTICK_RATE_MS);
	return ucExecRes;
}



uint8_t ucEsp_CIPCLOSE(void)
{
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT+CIPCLOSE\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}


uint8_t ucEsp_CIPMODE(const uint8_t ucMode)
{
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CIPMODE=%u\r\n", ucMode);
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}


uint8_t ucEsp_CIPSEND(const uint8_t ucLinkID, const uint32_t ulLength)
{
	const char *ppcESP_SuccRes[] = {">", NULL};
	
	uint8_t ucExecRes = 0x00;
	char pcCmd[128];
	
	if (ulLength != 0 && ucLinkID < 5)
	{
		//多连接，非透传
		memset(pcCmd, 0, sizeof(pcCmd));
		sprintf(pcCmd, "AT+CIPSEND=\"%d\",%d\r\n", ucLinkID, ulLength);
	}
	else if (ulLength != 0 && ucLinkID >= 5)
	{
		//单连接，非透传
		memset(pcCmd, 0, sizeof(pcCmd));
		sprintf(pcCmd, "AT+CIPSEND=%d\r\n", ulLength);
	}
	else
	{
		//透传模式
		sprintf(pcCmd, "AT+CIPSEND\r\n");
	}
	ucExecRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}


uint8_t ucEsp_BreakSEND(void)
{
	vTaskDelay(100/portTICK_RATE_MS);
	//必须是单独的一串+++，所以和前后数据要隔开一定的时间
	ucSerialAT_SendCmd("+++", 1000/portTICK_RATE_MS);
	vTaskDelay(100/portTICK_RATE_MS);
	return 0;
}


uint8_t ucEsp_WaitSendOK(void)
{
	uint8_t ucRes = 0x00;
	const char *ppcESP_SuccRes[] = {"SEND OK", NULL};
	
	ucRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucRes;
}













uint8_t ucEsp_CIPMUX(const uint8_t ucCipmux)
{
	uint8_t ucRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CIPMUX=%u\r\n", ucCipmux);
	ucRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucRes;
}


uint8_t ucEsp_CIPSERVER(const uint8_t ucMode, const uint32_t ulport)
{
	uint8_t ucRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	sprintf(pcCmd, "AT+CIPSERVER=%u,%u\r\n", ucMode, ulport);
	ucRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucRes;
}




uint8_t ucEsp_CIPSENDBUF(const uint8_t ucLinkID, const uint32_t ulLen)
{
	uint8_t ucRes = 0x00;
	char pcCmd[128];
	
	memset(pcCmd, 0, sizeof(pcCmd));
	if (ucLinkID >= 5)
	{
		sprintf(pcCmd, "AT+CIPSENDBUF=%u\r\n", ulLen);
	}
	else
	{
		sprintf(pcCmd, "AT+CIPSENDBUF=%u,%u\r\n", ucLinkID, ulLen);
	}
	ucRes |= ucSerialAT_SendCmd(pcCmd, 1000/portTICK_RATE_MS);
	ucRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucRes;
}


















uint8_t ucEsp_CWSTARTSMART(void)
{
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT+CWSTARTSMART\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}

uint8_t ucEsp_CWSTOPSMART(void)
{
	uint8_t ucExecRes = 0x00;
	
	ucExecRes |= ucSerialAT_SendCmd("AT+CWSTOPSMART\r\n", 1000/portTICK_RATE_MS);
	ucExecRes |= ucSerialAT_WaitRes(ppcESP_SuccRes, ppcESP_FailRes, 1000/portTICK_RATE_MS);
	return ucExecRes;
}

