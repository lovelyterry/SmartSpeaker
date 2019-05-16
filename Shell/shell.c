#include "shell.h"
#include <string.h>


void vShell_Task(void *pvParameters)
{
	static uint8_t argc = 0;
	static char *argv[MAX_ARGS] = {0};
	
	ucChangeEcho(0x01);
	ucChangePrompt("Shell: ");
	for (;;)
	{
		if (ulReadLine())
		{
			memset(&argc, 0, sizeof(argc));
			memset(argv, 0, sizeof(argv));
			if (!ucArgAnalyze(&argc,argv))
			{
				ucCmd_exec(argc, argv);
			}
		}
	}
}


