#ifndef __READ_LINE
#define __READ_LINE


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_ARGS	(0xFFU)
#define PROMPT_LEN	(0xFFU)
#define READBUF_LEN	(1024U)

uint32_t ulReadLine(void);
uint8_t ucArgAnalyze(uint8_t *argc, char *argv[]);
uint8_t ucChangePrompt(const char *cNewPro);
uint8_t ucChangeEcho(const uint8_t ucNewEcho);


#ifdef __cplusplus
}
#endif

#endif
