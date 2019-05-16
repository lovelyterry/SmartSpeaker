#ifndef __AUDIO_H
#define __AUDIO_H


#include "stdint.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"




#define LHP_DEF		(20U)
#define RHP_DEF		(20U)
#define SPK_DEF		(0X3FU)

extern uint8_t ucLHPvol;
extern uint8_t ucRHPvol;
extern uint8_t ucSPKvol;





//WM8978的音频收发缓冲区大小
#define AUDIO_BUFFER_SIZE		(1152*4U)

extern uint8_t pucAudio_SendBuf[AUDIO_BUFFER_SIZE*2];
extern uint8_t pucAudio_ReceiveBuf[AUDIO_BUFFER_SIZE*2];


#define pAUDIO_SEND_BUF0			((uint8_t *)(pucAudio_SendBuf))
#define pAUDIO_SEND_BUF1			((uint8_t *)(pucAudio_SendBuf + AUDIO_BUFFER_SIZE))

#define pAUDIO_RECEIVE_BUF0			((uint8_t *)(pucAudio_ReceiveBuf))
#define pAUDIO_RECEIVE_BUF1			((uint8_t *)(pucAudio_ReceiveBuf + AUDIO_BUFFER_SIZE))


void vAudio_Task(void *pvParameters);




#endif
