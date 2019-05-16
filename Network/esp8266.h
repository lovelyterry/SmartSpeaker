#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f4xx.h"

/*--------------------------------------------------------------*/
void vEsp_Init(void);
void vEsp_Reset(void);
void vEsp_LowPower(const uint8_t ucMode);
uint8_t ucEsp_DataExSpeedCfg(uint32_t ulSpeed);
uint8_t ucEsp_Debug(const char *pcSend);
/*--------------------------------------------------------------*/



/*--------------------------------------------------------------*/
uint8_t ucEsp_AT(void);
uint8_t ucEsp_ATE(const uint8_t ucEcho);
/*--------------------------------------------------------------*/



/*--------------------------------------------------------------*/
uint8_t ucEsp_RESTORE(void);
uint8_t ucEsp_CWMODE_CUR(const uint8_t ucMode);
uint8_t ucEsp_UART_CUR(const uint32_t ulBaudRate);
uint8_t ucEsp_CWSAP_CUR(const char *pSSID, const char *pPassWord, const uint8_t ucChl, const uint8_t ucEcn);
uint8_t ucEsp_CWJAP_CUR(const char *pSSID, const char *pPassWord);
uint8_t ucEsp_CWJAP(void);
uint8_t ucEsp_CWLAP(void);
uint8_t ucEsp_CIPSTART(const char *pcProtocol, const char* pcIpAddr, const uint16_t usPort);
uint8_t ucEsp_CIPCLOSE(void);
uint8_t ucEsp_CIPMODE(const uint8_t ucMode);
uint8_t ucEsp_CIPSEND(const uint8_t ucLinkID, const uint32_t ulLength);

uint8_t ucEsp_BreakSEND(void);
uint8_t ucEsp_WaitSendOK(void);

uint8_t ucEsp_CIPMUX(const uint8_t ucCipmux);
uint8_t ucEsp_CIPSERVER(const uint8_t ucMode, const uint32_t ulport);
uint8_t ucEsp_CIPSENDBUF(const uint8_t ucLinkID, const uint32_t ulLen);




uint8_t ucEsp_CWSTARTSMART(void);
uint8_t ucEsp_CWSTOPSMART(void);
/*--------------------------------------------------------------*/


#endif
