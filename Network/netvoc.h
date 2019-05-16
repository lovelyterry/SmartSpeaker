#ifndef __NETVOC_H
#define __NETCOC_H

#include <stdint.h>


uint8_t ucNet_ReadToken(void);
void vNet_ReacquireToken(void);

uint8_t ucNet_Audio2Text(const char *pcfile);
void vNet_Text2Audio(const char *pcText);
uint8_t ucNet_GetText(char* pcText);


#endif

