#include "cmd.h"
#include "read-line.h"
#include "syslog.h"


#include <string.h>




static const cmd_t cmd_array[];


uint8_t ucCmd_exec(uint8_t argc, char **argv)
{
	static uint32_t ulNFcnt = 0;
	static uint32_t ulEPcnt = 0;

	if (argc != 0 && argv != NULL)
	{
		for (uint8_t i=0; cmd_array[i].cmd_name != NULL; i++)
		{
			if(strcmp(cmd_array[i].cmd_name, argv[0]) == 0)
			{
				if(cmd_array[i].cmd_func != NULL)
				{
					//TODO:write log
					(cmd_array[i].cmd_func)(argc, argv);
					return 0;
				}
			}
		}
		ts_printf("没有找到这条命令，请检查你的输入或输入\"help\"查看所有命令以及帮助信息。\r\n");
		ulNFcnt++;
		return 1;
	}
	else
	{
		ulEPcnt++;
		return 1;
	}
}



static uint8_t prvCmd_printarg(uint8_t argc, char **argv)
{
	for (uint8_t i=0; i<argc; i++)
	{
		ts_printf("%d: %s \r\n", i, argv[i]);
	}
	return 0;
}

static uint8_t prvCmd_help(uint8_t argc, char **argv)
{
	(void) argc;
	(void) argv;

	ts_printf("------------------------------------------------------------------\r\n");
	for (uint8_t i=0; cmd_array[i].cmd_name != NULL; i++)
	{
		ts_printf("    %s  %s \r\n", cmd_array[i].cmd_name, cmd_array[i].cmd_help);
	}
	ts_printf("------------------------------------------------------------------\r\n");
	return 0;
}


static uint8_t prvCmd_setpro(uint8_t argc, char **argv)
{
	if (argc == 1)
	{
		ucChangePrompt("\0");
	}
	else if (argc == 2)
	{
		ucChangePrompt(argv[1]);
	}
	else
	{
		ts_printf("参数格式错误！\r\n");
	}
	return 0;
}



static uint8_t prvCmd_echo(uint8_t argc, char **argv)
{
	if (argc == 2)
	{
		if(strcmp("-on", argv[1]) == 0)
		{
			ucChangeEcho(0x01);
		}
		if(strcmp("-off", argv[1]) == 0)
		{
			ucChangeEcho(0x00);
		}
	}
	else
	{
		ts_printf("参数格式错误！\r\n");
	}
	return 0;
}


//---------------------------------------------------------------------
#include "usart.h"
static uint8_t prvCmd_tapping(uint8_t argc, char **argv)
{
#ifndef __TAPPING_MODE
	(void) argc;
	(void) argv;

	ts_printf("暂不支持窃听模式，请在代码中开启！\r\n");
	return 0;
#else
	if (argc == 1)
	{
		if(ucTapping == 0x00)
		{
			ts_printf("窃听已关闭\r\n");
		}
		else
		{
			ts_printf("窃听已开启\r\n");
		}
		return 0;
	}
	else if (argc == 2)
	{
		if(strcmp("-on", argv[1]) == 0)
		{
			ucTapping = 0x01;
		}
		if(strcmp("-off", argv[1]) == 0)
		{
			ucTapping = 0x00;
		}
	}
	else
	{
		ts_printf("参数格式错误！\r\n");
	}
	return 0;
#endif
}
//---------------------------------------------------------------------

//---------------------------------------------------------------------
#include "esp8266.h"
static uint8_t prvCmd_espat(uint8_t argc, char **argv)
{
	if (argc == 2)
	{
		ucEsp_Debug(argv[1]);
	}
	else
	{
		ts_printf("参数格式错误！\r\n");
	}
	return 0;
}
//---------------------------------------------------------------------


//---------------------------------------------------------------------
#include <stdlib.h>
#include "audio.h"
#include "wm8978.h"


static uint8_t prvCmd_volume(uint8_t argc, char **argv)
{
	char *pcSH = NULL;
	char *pcSS = NULL;
	uint8_t ucVol = 0;
	
	if (argc >= 2)
	{
		for (uint8_t i=1; i<argc; i++)
		{
			pcSH = strstr(argv[i], "HP=");
			pcSS = strstr(argv[i], "SPK=");
			
			if (pcSH)
			{
				if (strlen(argv[i]) >5)
				{
					break;
				}
				ucVol = atoi((char *)(pcSH+3));
				if (ucVol>0 && ucVol<0x40)
				{
					ucLHPvol = ucVol;
					ucRHPvol = ucVol;
					WM8978_HPvol_Set(ucLHPvol,ucRHPvol);
				}
			}
			else if (pcSS)
			{
				if (strlen(argv[i]) >6)
				{
					break;
				}
				ucVol = atoi((char *)(pcSS+4));
				if (ucVol>0 && ucVol<0x40)
				{
					ucSPKvol = ucVol;
					WM8978_SPKvol_Set(ucSPKvol);
				}
			}
		}
	}
	else
	{
		ts_printf("参数格式错误！\r\n");
	}
	return 0;
}
//---------------------------------------------------------------------



static const cmd_t cmd_array[] =
{
	{"printarg", "  --打印出你输入的所有参数。", prvCmd_printarg},
	{"help", "      --打印出所有的命令以及提示信息。", prvCmd_help},
	{"setpro", "    --修改命令行提示信息。 \
参数：Shell: ", prvCmd_setpro},
	{"echo", "      --开启或关闭字符回显模式，建议开启。\
参数：-on 打开回显模式， -off 关闭回显模式。", prvCmd_echo},
	{"tapping", "   --开启或关闭窃听模式，用于调试串口2上的8266。\
参数：-on 打开窃听模式， -off 关闭窃听模式。", prvCmd_tapping},
	{"espat", "     --将字符串发送到串口2上的8266，会自动添加回车。\
参数：AT+xxxx", prvCmd_espat},
	{"volume", "    --设置音量。\
参数：设置耳机音量：HP=xx,设置喇叭音量：SPK=xx", prvCmd_volume},


	{NULL, NULL, NULL}
};



