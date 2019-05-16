#include "WM8978.h"
#include "soft_iic.h"

/**
 *由于WM8978的IIC接口不支持读取操作，因此寄存器值缓存在内存中当写寄存器时同步更新缓存，
 *当你想读寄存器时直接返回缓存中的值。
*/
static uint16_t WM8978_RegCash[] =
{
	0x0000,0x0000,0x0000,0x0000,0x0050,0x0000,0x0140,0x0000,
	0x0000,0x0000,0x0000,0x00FF,0x00FF,0x0000,0x0100,0x00FF,
	0x00FF,0x0000,0x012C,0x002C,0x002C,0x002C,0x002C,0x0000,
	0x0032,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0038,0x000B,0x0032,0x0000,0x0008,0x000C,0x0093,0x00E9,
	0x0000,0x0000,0x0000,0x0000,0x0003,0x0010,0x0010,0x0100,
	0x0100,0x0002,0x0001,0x0001,0x0039,0x0039,0x0039,0x0039,
	0x0001,0x0001
};

uint16_t WM8978_ReadReg(uint8_t RegAddr)
{
	return WM8978_RegCash[RegAddr];
}

uint8_t WM8978_WriteReg(uint8_t RegAddr, uint16_t Value)
{
	//时钟线高电平时拉低数据线，启动IIC传输，然后拉低时钟线
	vSoftIIC_Start();
	//发送IIC从机地址，最后一位是0，表示写入数据
	if (xSoftIIC_WriteByte(WM8978_Addr | IIC_WR))
	{
		return 1;
	}
	//地址7bit 指令9bit，先发送最高的第一个bit
	if (xSoftIIC_WriteByte(((RegAddr << 1) & 0xFE) | ((Value >> 8) & 0x1)))
	{
		return 1;
	}
	if (xSoftIIC_WriteByte(Value & 0xFF))
	{
		return 1;
	}
	vSoftIIC_Stop();
	//更新内存中的缓存
	WM8978_RegCash[RegAddr] = Value;
	return 0;
}

void WM8978_SetVolume(uint8_t Volume)
{
	uint16_t regV;
	
	Volume &= 0x3F;
	regV = Volume;
	//耳机左右声道音量设置
	WM8978_WriteReg(52, regV);
	WM8978_WriteReg(53, regV|(1<<8));//同步更新(SPKVU=1)
	//喇叭左右声道音量设置
	WM8978_WriteReg(54, regV);
	WM8978_WriteReg(55, regV|(1<<8));
}

uint8_t WM8978_ReadVolume(void)
{
	return (uint8_t)(WM8978_ReadReg(52)&0x3F);
}

void WM8978_MIC_Gain(uint8_t Gain)
{
	Gain &= 0x3F;
	WM8978_WriteReg(45, Gain);
	WM8978_WriteReg(46, Gain|(1<<8));
}

/**
 *设置I2S工作模式
 *fmt:0,LSB(右对齐);1,MSB(左对齐);2,飞利浦标准I2S;3,PCM/DSP;
 *len:0,16位;1,20位;2,24位;3,32位;
*/
void WM8978_I2S_Cfg(uint8_t fmt,uint8_t len)
{
	//限定范围
	fmt &= 0x03;
	len &= 0x03;
	//R4,WM8978工作模式设置
	WM8978_WriteReg(4,(fmt<<3)|(len<<5));
}

/**
 *WM8978 DAC/ADC配置
 *adcen:adc使能(1)/关闭(0)
 *dacen:dac使能(1)/关闭(0)
*/
void WM8978_ADDA_Cfg(uint8_t dacen,uint8_t adcen)
{
	uint16_t regval;
	
	regval=WM8978_ReadReg(3);
	if(dacen)
	{
		//R3最低2个位设置为1,开启DACR&DACL
		regval|=3<<0;
	}
	else
	{
		//R3最低2个位清零,关闭DACR&DACL.
		regval&=~(3<<0);
	}
	WM8978_WriteReg(3,regval);
	regval=WM8978_ReadReg(2);
	if(adcen)
	{
		//R2最低2个位设置为1,开启ADCR&ADCL
		regval|=3<<0;
	}
	else
	{
		//R2最低2个位清零,关闭ADCR&ADCL.
		regval&=~(3<<0);
	}
	WM8978_WriteReg(2,regval);
}

/**
 *WM8978 L2/R2(也就是Line In)增益设置(L2/R2-->ADC输入部分的增益)
 *gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
*/
void WM8978_LINEIN_Gain(uint8_t gain)
{
	uint16_t regval;
	
	gain&=0X07;
	regval=WM8978_ReadReg(47);
	//清除原来的设置
	regval&=~(7<<4);
	WM8978_WriteReg(47,regval|gain<<4);
	regval=WM8978_ReadReg(48);
	//清除原来的设置
	regval&=~(7<<4);
	WM8978_WriteReg(48,regval|gain<<4);
}

/**
 *WM8978 AUXR,AUXL(PWM音频部分)增益设置(AUXR/L-->ADC输入部分的增益)
 *gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
*/
void WM8978_AUX_Gain(uint8_t gain)
{
	uint16_t regval;
	
	gain&=0X07;
	regval=WM8978_ReadReg(47);
	//清除原来的设置
	regval&=~(7<<0);
	WM8978_WriteReg(47,regval|gain<<0);	
	regval=WM8978_ReadReg(48);
	//清除原来的设置
	regval&=~(7<<0);
	WM8978_WriteReg(48,regval|gain<<0);
}

/*
 *WM8978 输入通道配置
 *micen:MIC开启(1)/关闭(0)
 *lineinen:Line In开启(1)/关闭(0)
 *auxen:aux开启(1)/关闭(0)
*/
void WM8978_Input_Cfg(uint8_t micen,uint8_t lineinen,uint8_t auxen)
{
	uint16_t regval;
	
	regval=WM8978_ReadReg(2);
	if(micen)
	{
		//开启INPPGAENR,INPPGAENL(MIC的PGA放大)
		regval|=3<<2;
	}
	else
	{
		//关闭INPPGAENR,INPPGAENL.
		regval&=~(3<<2);
	}
	WM8978_WriteReg(2,regval);
	regval=WM8978_ReadReg(44);
	if(micen)
	{
		//开启LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
		regval|=3<<4|3<<0;
	}
	else
	{
		//关闭LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
		regval&=~(3<<4|3<<0);
	}
	WM8978_WriteReg(44,regval);
	if(lineinen)
	{
		//LINE IN 0dB增益
		WM8978_LINEIN_Gain(5);
	}
	else
	{
		//关闭LINE IN
		WM8978_LINEIN_Gain(0);
	}
	if(auxen)
	{
		//AUX 6dB增益
		WM8978_AUX_Gain(7);
	}
	else
	{
		//关闭AUX输入
		WM8978_AUX_Gain(0);
	}
}

/**
 *WM8978 输出配置
 *dacen:DAC输出(放音)开启(1)/关闭(0)
 *bpsen:Bypass输出(录音,包括MIC,LINE IN,AUX等)开启(1)/关闭(0)
*/
void WM8978_Output_Cfg(uint8_t dacen,uint8_t bpsen)
{
	uint16_t regval=0;
	if(dacen)
	{
		//DAC输出使能
		regval|=1<<0;
	}
	if(bpsen)
	{
		//BYPASS使能
		regval|=1<<1;
		//0dB增益
		regval|=5<<2;
	}
	WM8978_WriteReg(50,regval);
	WM8978_WriteReg(51,regval);
}

/**
 *设置耳机左右声道音量
 *voll:左声道音量(0~63)
 *volr:右声道音量(0~63)
*/
void WM8978_HPvol_Set(uint8_t voll,uint8_t volr)
{
	//限定范围
	voll&=0X3F;
	volr&=0X3F;
	//音量为0时,直接mute
	if(voll==0)
	{
		voll|=1<<6;
	}
	//音量为0时,直接mute
	if(volr==0)
	{
		volr|=1<<6;
	}
	//R52,耳机左声道音量设置
	WM8978_WriteReg(52,voll);
	//R53,耳机右声道音量设置,同步更新(HPVU=1)
	WM8978_WriteReg(53,volr|(1<<8));
}

/**
 *设置喇叭音量
 *voll:左声道音量(0~63)
*/
void WM8978_SPKvol_Set(uint8_t volx)
{
	//限定范围
	volx&=0x3F;
	
	//音量为0时,直接mute
	if(volx==0)volx|=1<<6;
	//R54,喇叭左声道音量设置
	WM8978_WriteReg(54,volx);
	//R55,喇叭右声道音量设置,同步更新(SPKVU=1)
	WM8978_WriteReg(55,volx|(1<<8));
}


uint8_t WM8978_Init(void)
{
	uint8_t ucFun_res = 0;
	
	vSoftIIC_Init();
	ucFun_res |= WM8978_WriteReg(0,0x0000);	//向0寄存器写入任意值，就可以软件复位
	//先把声音关到最小，防止噪音
	WM8978_HPvol_Set(0,0);
	WM8978_SPKvol_Set(0);
	ucFun_res |= WM8978_WriteReg(1,0x001B);	//R1,MICEN设置为1(MIC使能),BIASEN设置为1(模拟器工作),VMIDSEL[1:0]设置为:11(5K)
	ucFun_res |= WM8978_WriteReg(2,0x01B0);	//R2,ROUT1,LOUT1输出使能(耳机可以工作),BOOSTENR,BOOSTENL使能
	ucFun_res |= WM8978_WriteReg(3,0x006C);	//R3,LOUT2,ROUT2输出使能(喇叭工作),RMIX,LMIX使能
	ucFun_res |= WM8978_WriteReg(6,0x0000);	//R6,MCLK由外部提供

	ucFun_res |= WM8978_WriteReg(43,1<<4);	//R43,INVROUT2反向,驱动喇叭
	ucFun_res |= WM8978_WriteReg(47,1<<8);	//R47设置,PGABOOSTL,左通道MIC获得20倍增益
	ucFun_res |= WM8978_WriteReg(48,1<<8);	//R48设置,PGABOOSTR,右通道MIC获得20倍增益
	ucFun_res |= WM8978_WriteReg(49,1<<1);	//R49,TSDEN,开启过热保护
	ucFun_res |= WM8978_WriteReg(10,1<<3);	//R10,SOFTMUTE关闭,128x采样,最佳SNR
	ucFun_res |= WM8978_WriteReg(14,1<<3);	//R14,ADC 128x采样率
	
	return ucFun_res;
}

void WM8978_PlayMode(void)
{
	WM8978_ADDA_Cfg(1,0);		//开启DAC
	WM8978_Input_Cfg(0,0,0);	//关闭输入通道
	WM8978_Output_Cfg(1,0);		//开启DAC输出
	WM8978_HPvol_Set(20,20);
	WM8978_SPKvol_Set(0x3F);
	WM8978_MIC_Gain(0);
	WM8978_I2S_Cfg(2,0);		//设置I2S接口模式
}

void WM8978_RecoMode(void)
{
	WM8978_ADDA_Cfg(0,1);		//开启ADC
	WM8978_Input_Cfg(1,0,0);	//开启输入通道(MIC)
	WM8978_Output_Cfg(0,1);		//开启BYPASS输出
	WM8978_HPvol_Set(0,0);
	WM8978_SPKvol_Set(0);
	WM8978_MIC_Gain(50);		//MIC增益设置
	WM8978_I2S_Cfg(2,0);		//飞利浦标准,16位数据长度
}
