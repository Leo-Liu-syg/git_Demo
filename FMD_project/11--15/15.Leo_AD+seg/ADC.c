
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "TDelay.h"
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

/*-------------------------------------------------
 * 函数名：SPI_INITIAL
 * 功能：  初始化IIC
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void ADC_INITIAL(void)
{
	ANSELA = 0B00000001; // 控制IO的数模输入，1：对应的IO为模拟管脚，0：对应的IO为数字IO，设置AN0为模拟管脚
	ADCON0 = 0B00000000;
	// Bit[6:4]:000-选择模拟通道AN0
	// Bit2:当软件设定GO/DONE位时启动AD转换（即不用外部触发源）

	ADCON1 = 0B11100100;
	// Bit7:	1-ADC转换结果右对齐，即装入转换结果时，ADRESH的高4位被设置为0
	// Bit[6:4]:110-ADC转换时钟设置为Fosc/64
	// Bit[3:2]:01-负参考电压-GND
	// Bit[1:0]:00-正参考电压-内部参考电压

	ADCON2 = 0B01000000;
	// Bit[7:6]:01-ADC内部参考电压2V

	ADCON3 = 0B00000000;
	ADDLY = 0B00000000;	 // 外部触发延时
	ADCMPH = 0B00000000; // ADC比较阈值，用于ADC结果高8位比较
	ADON = 1;			 // 使能ADC

	Delay450Us(); // 打开ADC模块后，需等待ADC稳定时间Tst(~15us);当选择内部参考电压时需等待内部参考电压的稳定时间Tvrint(~450us)
}

uint GET_ADC_DATA(uchar adcChannel)
{
	ADCON0 &= 0B00001111;
	ADCON0 |= adcChannel << 4;
	Delay10Us(); // TACQ延时2us,外部串联电阻小于21kΩ
				 // TACQ延时4us,外部串联电阻43kΩ
	// TACQ时间：必做，通道切换到GO/DONE置1的时间,保证内部 ADC 输入电容充满。
	// TACQ > 0.09*(R+1)us;R为外部串联电阻(kΩ),串联电阻越小越好，最大不要超过50kΩ
	GO = 1;
	NOP(); // 采样保持时间0~1TAD
	NOP();
	while (GO)
		;
	// 从GO = 1 ---> 从GO = 0,转换过程需要16TAD
	// TAD(us)与转换时钟Fosc/ADCS[2:0]有关
	return (uint)(ADRESH << 8 | ADRESL);
}

