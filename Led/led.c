#include "led.h"

//0为正常的状态，1为频谱指示
uint8_t exucLED_Sta = 0;




static void prvLed_OB_IOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


static void prvLed_EX_IOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);//红
	GPIO_SetBits(GPIOA, GPIO_Pin_7);//绿

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);//蓝
}



void vLedPWM_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);

	TIM_TimeBaseStructure.TIM_Period = 1000;		//1ms(周期)
	TIM_TimeBaseStructure.TIM_Prescaler = 81;		//定时器分频系数，定时器时钟为82MHz，分频后得1MHz,即1000kHz,0.001ms定时器加1
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//PWM1为正常占空比模式，PWM2为反极性模式
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 500;//输入CCR（占空比数值）
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);//CCR自动装载默认也是打开的
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE); //ARR自动装载默认是打开的，可以不设置
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}


#include "arm_math.h"
#include "audio.h"
#include "syslog.h"

#define FFT_LENGTH		64 		//FFT长度，默认是1024点FFT

float32_t fft_inputbuf[FFT_LENGTH*2];	//FFT输入数组
float32_t fft_outputbuf[FFT_LENGTH];	//FFT输出数组


void vLed_Task(void *pvParameters)
{
	prvLed_OB_IOInit();
	prvLed_EX_IOInit();
	for(;;)
	{
		if (exucLED_Sta == 1)
		{
			arm_cfft_radix4_instance_f32 S;
			uint16_t msctmp = 0;

			vLedPWM_Init();
			for (uint32_t i=0; i<FFT_LENGTH; i++)
			{
				msctmp = *((uint16_t *)pucAudio_SendBuf + 2*i);

				fft_inputbuf[i*2] = (float32_t)msctmp;
				fft_inputbuf[i*2+1] = 0;
			}
			arm_cfft_radix4_init_f32(&S, FFT_LENGTH, 0, 1);
			arm_cfft_radix4_f32(&S, fft_inputbuf);
			arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);

			float32_t minval = 0;
			for(uint32_t i=1; i<FFT_LENGTH/3*1; i++)
			{
				minval += fft_outputbuf[i]/FFT_LENGTH*2;
			}

			float32_t medval = 0;
			for(uint32_t i=FFT_LENGTH/3*1; i<FFT_LENGTH/3*2; i++)
			{
				medval += fft_outputbuf[i]/FFT_LENGTH*2;
			}

			float32_t maxval = 0;
			for(uint32_t i=FFT_LENGTH/3*2; i<FFT_LENGTH/3*3; i++)
			{
				maxval += fft_outputbuf[i]/FFT_LENGTH*2;
			}
			//ts_printf("%f %f %f \r\n", minval, medval, maxval);
			//红绿蓝
			TIM_SetCompare1(TIM3, maxval/200 > 1000 ? 1000 : maxval/200);
			TIM_SetCompare2(TIM3, medval/200 > 1000 ? 1000 : medval/200);
			TIM_SetCompare4(TIM3, minval/200 > 1000 ? 1000 : minval/200);
			vTaskDelay(50);
		}
		else if (exucLED_Sta == 0)
		{
			vTaskDelay(1000);
		}
		else
		{
			prvLed_EX_IOInit();
			exucLED_Sta = 0;
		}
	}
}


