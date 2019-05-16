#ifndef __PLAYER_H
#define __PLAYER_H

#include <stdint.h>
#include "ff.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#define MUSIC_FOLDER		"0:/music"
#define MAX_DIR_SIZE		(512U)


void vPlayer_Task(void *pvParameters);



//取2个值里面的较小值.
#ifndef AUDIO_MIN
#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))
#endif


#endif


