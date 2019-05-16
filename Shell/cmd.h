#ifndef __CMD_H
#define __CMD_H


#include <stdint.h>

typedef uint8_t (ucCmdFun_t)(uint8_t argc, char **argv);


typedef struct
{
	char *cmd_name;
	char *cmd_help;
	ucCmdFun_t *cmd_func;
} cmd_t;


uint8_t ucCmd_exec(uint8_t argc, char **argv);



#endif


