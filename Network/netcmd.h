#ifndef __NETCMD_H
#define __NETCMD_H


#include <stdint.h>
#include <stddef.h>

typedef uint8_t (ucNetCmdFunc_t)(uint8_t argc, char **argv);


typedef struct
{
	ucNetCmdFunc_t *cmd_func;
	char *key_word[16];
} netcmd_t;



uint8_t ucParseWord(uint8_t argc, char **argv);





#endif

