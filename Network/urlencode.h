#ifndef __URLENCODE_H
#define __URLENCODE_H

#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif

uint32_t ulURL_Encode(const char *pcUtf_8, uint32_t ulNum, char *pcUrl);


uint32_t ulUnicodetoUtf_8(const char *pcUincode, uint32_t ulNum, char *pcUtf_8);
uint32_t ulUtf_8toUnicode(const char *pcUtf_8, uint32_t ulNum, char *pcUincode);
uint32_t UnicodetoGbk(const char *pcUincode, uint32_t ulNum, char *pcGbk);
uint32_t GbktoUnicode(const char *pcGbk, uint32_t ulNum, char *pcUincode);

#ifdef __cplusplus
}
#endif /* end of __cplusplus */


#endif
