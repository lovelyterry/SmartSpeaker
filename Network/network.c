#include "network.h"
#include "player.h"
#include "bsp_init.h"
#include "urlencode.h"
#include "netcmd.h"
#include "syslog.h"
#include <string.h>



uint8_t pucNet_SendBuf[NET_BUFFER_SIZE*2] = {0};
uint8_t pucNet_ReceiveBuf[NET_BUFFER_SIZE*2] = {0};

SemaphoreHandle_t xNetStart_Binary = NULL;
SemaphoreHandle_t xNetEnd_Binary = NULL;


static char SSID_gbk[128] = {"YourSSID"};
static char PSWD[64] = {"YourPassWord"};


#if 0
//IDE和网络的编码都是UTF-8的，如果使用中文的SSID实际使用需要转换成GBK的格式
static char SSID_utf8[128] = {""};
//函数说明，将utf-8的SSID转换为GBK格式
static void vSSID_ChangeCode(void)
{
	char temp[128] = {0};
	uint8_t unilen = 0;		//unicode可能中间包含有0x00，所以不能strlen;
	
	unilen = ulUtf_8toUnicode(SSID_utf8, strlen(SSID_utf8), temp);
	UnicodetoGbk(temp, unilen, SSID_gbk);
}
#endif




void vNet_Task(void *pvParameters)
{
	static char VocText[128] = {0};
	
	vNetLsn_Init();
	vSemaphoreCreateBinary(xNetStart_Binary);
	vSemaphoreCreateBinary(xNetEnd_Binary);
	xSemaphoreTake(xNetStart_Binary, 0);
	
net_reset:
	ts_printf("正在初始化网络连接...\r\n");
	vEsp_Init();
	if ( ucEsp_CWMODE_CUR(1) ){goto net_reset;}
	if ( ucEsp_CWJAP_CUR(SSID_gbk, PSWD) ) {goto net_reset;}
	vNet_ReacquireToken();
	if ( ucNet_ReadToken() ) {goto net_reset;}
	ts_printf("网络连接成功！\r\n");
	for (;;)
	{
		xSemaphoreGive(xNetEnd_Binary);
		xSemaphoreTake(xNetStart_Binary, portMAX_DELAY);
		if ( ucEsp_CWJAP() ) {goto net_reset;}//检查网络连接是否正常
		ts_printf("正在识别中...\r\n");
		if ( ucNet_Audio2Text("0:/SmartSpeaker/record") ) {goto net_reset;}
		memset(VocText, 0, sizeof (VocText));
		if ( ucNet_GetText(VocText) == 0 )
		{
			ts_printf("识别结果：%s \r\n",VocText);
			ucParseWord(1, (char **)(&VocText));
		}
		else
		{
			ts_printf("识别失败！\r\n");
		}
	}
}



