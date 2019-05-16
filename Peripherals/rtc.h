#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx.h"




uint8_t ucRtc_Init(void);
void RTC_Set_WakeUp(uint32_t wksel,uint16_t cnt);
ErrorStatus RTC_Set_Time(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm);
ErrorStatus RTC_Set_Date(uint8_t year,uint8_t month,uint8_t date,uint8_t week);
void RTC_Set_AlarmA(uint8_t week,uint8_t hour,uint8_t min,uint8_t sec);
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);
void RTC_Get_Time(uint8_t *hour,uint8_t *min,uint8_t *sec,uint8_t *ampm);
void RTC_Get_Date(uint8_t *year,uint8_t *month,uint8_t *date,uint8_t *week);

#endif

