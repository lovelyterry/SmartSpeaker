#include "netvoc.h"

#include "esp8266.h"
#include "netlsn.h"
#include "network.h"
#include "urlencode.h"


#include <string.h>
#include <stdio.h>

#include "syslog.h"

//请注册百度语音开发者账号，将下面的xxxxxxxx替换为自己申请的ID。

static const char __get_token[] = 
{
"GET /oauth/2.0/token?\
grant_type=client_credentials\
&client_id=xxxxxxxxxxxxxx\
&client_secret=xxxxxxxxxxxxxxxx \
HTTP/1.1\r\nHost: openapi.baidu.com\r\n\r\n"
};


static const char __audio2text[] = 
{
"POST /server_api?\
lan=zh\
&cuid=xxxxxxxx\
&token=%s \
HTTP/1.1\r\nHost: vop.baidu.com\r\n\
Content-Type: wav;rate=8000\r\nContent-Length: %s\r\n\r\n"
};

static const char __text2audio[] = 
{
"GET /text2audio?\
tex=%s\
&lan=zh\
&cuid=001\
&ctp=1\
&per=4\
&tok=%s \
HTTP/1.1\r\nHost: tsn.baidu.com\r\n\r\n"
};



#include "bsp_init.h"
#define __TOKEN_SIZE	(71U)
static char pcTokenBuf[__TOKEN_SIZE+2] = {0};


uint8_t ucNet_ReadToken(void)
{
	//若http响应正确
	if (strstr((char *)PubBuf, "HTTP/1.1 200 OK\r\n") != NULL)
	{
		char *pT = NULL;
		char *pS = NULL;
		char *pE = NULL;
		
		pT = strstr((char *)PubBuf, "\"refresh_token\":");
		pS = strstr(pT, ":\"");
		pE = strstr(pS, "\",");
		if ((pE - pS) == __TOKEN_SIZE+2)
		{
			strncpy(pcTokenBuf, (char *)(pS+2), __TOKEN_SIZE);
			return 0;
		}
	}
	return 1;
}




void vNet_ReacquireToken(void)
{
	//连接证书服务器
	ucEsp_CIPSTART("TCP", "openapi.baidu.com", 80);
	//开启透传模式
	ucEsp_CIPMODE(1);
	//准备发送数据
	ucEsp_CIPSEND(0,0);
	//把信号量置零
	xSemaphoreTake(xNetLsn_RecBinary, 0);
	vNet_SendBuf(__get_token, sizeof (__get_token));
	vNet_LsnStart(10000);
	//等待数据接收完毕
	xSemaphoreTake(xNetLsn_RecBinary, portMAX_DELAY);
	//发送关闭监听信号量
	vNetLsn_Close();
	//退出透传模式
	ucEsp_BreakSEND();
	//关闭透传
	ucEsp_CIPMODE(0);
	//断开和服务器的连接
	ucEsp_CIPCLOSE();
}




//最多支持64个汉字(64*2*3)
#define URL_BUF_SIZE				(384U)
static char pcNetUrl[URL_BUF_SIZE] = {0};

//文字转语音接口，输入参数UTF-8格式的汉字串
void vNet_Text2Audio(const char *pcText)
{
	memset(pcNetUrl, 0, sizeof (pcNetUrl));
	ulURL_Encode(pcText, \
				(strlen(pcText)*3 < URL_BUF_SIZE) ? strlen(pcText) : URL_BUF_SIZE, \
				pcNetUrl);
	//填充字段到发送缓冲区
	memset(pNET_SEND_BUF0, 0, NET_BUFFER_SIZE);
	snprintf((char *)pNET_SEND_BUF0, NET_BUFFER_SIZE, __text2audio, pcNetUrl, pcTokenBuf);
	
	ucEsp_CIPSTART("TCP", "tsn.baidu.com", 80);
	ucEsp_CIPMODE(1);
	ucEsp_CIPSEND(0,0);
	xSemaphoreTake(xNetLsn_RecBinary, 0);
	vNet_SendBuf((char *)pNET_SEND_BUF0, strlen((char *)pNET_SEND_BUF0));
	vNet_LsnStart(10000);
	xSemaphoreTake(xNetLsn_RecBinary, portMAX_DELAY);
	vNetLsn_Close();
	ucEsp_BreakSEND();
	ucEsp_CIPMODE(0);
	ucEsp_CIPCLOSE();
}



#include "ff.h"
//语音文件的文件指针(不使用动态分配)
static FIL redAu_fp = {0};
//每次读取的文件大小
static uint32_t ulrByte = 0;
//语音的文件大小的字符串
static char pcConLen[16] = {0};


uint8_t ucNet_Audio2Text(const char *pcfile)
{
	if (f_open(&redAu_fp, pcfile, FA_READ) == FR_OK)
	{
		memset(pcConLen, 0, sizeof(pcConLen));
		snprintf(pcConLen, sizeof(pcConLen)-1, "%lld", f_size(&redAu_fp));
		
		memset((char *)pNET_SEND_BUF0, 0, NET_BUFFER_SIZE);
		sprintf((char *)pNET_SEND_BUF0, __audio2text, pcTokenBuf, pcConLen);
		xSemaphoreTake(xNetLsn_RecBinary, 0);
		ucEsp_CIPSTART("TCP", "vop.baidu.com", 80);
		
		//因为涉及到大文件的传输，使用透传会发生数据丢失
		ucEsp_CIPMODE(0);
		ucEsp_CIPSEND(0xFF, strlen((char *)pNET_SEND_BUF0));
		vNet_SendBuf((char *)pNET_SEND_BUF0, strlen((char *)pNET_SEND_BUF0));
		if (ucEsp_WaitSendOK())
		{
			f_close(&redAu_fp);
			ucEsp_CIPCLOSE();
			return 2;
		}
		for (;;)
		{
			memset(pNET_SEND_BUF0, 0, NET_BUFFER_SIZE);
			f_read(&redAu_fp, pNET_SEND_BUF0, NET_BUFFER_SIZE, &ulrByte);
			ucEsp_CIPSENDBUF(0xFF, ulrByte);
			vNet_SendBuf((char *)pNET_SEND_BUF0, ulrByte);
			if (ucEsp_WaitSendOK())
			{
				f_close(&redAu_fp);
				ucEsp_CIPCLOSE();
				return 2;
			}
			if(ulrByte < NET_BUFFER_SIZE)
			{
				break;
			}
		}
		vNet_LsnStart(10000);
		f_close(&redAu_fp);
		xSemaphoreTake(xNetLsn_RecBinary, portMAX_DELAY);
		ucEsp_CIPCLOSE();
		return 0;
	}
	else
	{
		ts_printf("文件系统异常，请检查！");
		f_close(&redAu_fp);
		return 1;
	}
}



uint8_t ucNet_GetText(char *pcText)
{
	//若http响应正确
	if (strstr((char *)PubBuf, "HTTP/1.1 200 OK\r\n") != NULL)
	{
		//若识别成功
		if (strstr((char *)PubBuf, "\"err_no\":0,") != NULL)
		{
			char *pT = NULL;
			char *pS = NULL;
			char *pE = NULL;
			
			pT = strstr((char *)PubBuf, "\"result\":");
			pS = strstr(pT, "[");
			pE = strstr(pS, "],");
			if (pS != NULL && pE != NULL)
			{
				strncpy(pcText, (char *)(pS+2), (pE-pS-6));
				return 0;
			}
		}
	}
	return 1;
}


