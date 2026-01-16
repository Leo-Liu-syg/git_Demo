// Project: TEST_64F0Ax_TIM2_PWM.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description: TIM2 CH1,CH2输出周期为32kHz的方波
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-5        修改系统时钟为8MHz，使能LVR
//                FT64F0A5  TSSOP20
//             -------------------
// TIM2_CH1----|1(PA5)   	(PA4)20|-----TIM2_CH2
// NC----------|2(PA6)   	(PA3)19|-----------NC
// NC----------|3(PA7)   	(PA2)18|-----------NC
// NC----------|4(PC0)   	(PA1)17|-----------NC
// NC----------|5(PC1)		(PA0)16|-----------NC
// NC----------|6(PB7)		(PB0)15|-----------NC
// GND---------|7(GND)		(PB1)14|-----------NC
// NC----------|8(PB6)		(PB2)13|-----------NC
// VDD---------|9(VDD)		(PB3)12|-----------NC
// NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
//
//*********************************************************
#include "SYSCFG.h";
#include "FT64F0AX.h";
#include "delay.h";

//***********************宏定义****************************
#define DemoPortOut PB3

// Variable definition
volatile char W_TMP @0x70;	 // 系统占用不可以删除和修改
volatile char BSR_TMP @0x71; // 系统占用不可以删除和修改
unsigned char TempH1;
unsigned char TempL1;
unsigned int ComValue1;
unsigned int T1; // 1的周期

unsigned char TempH2;
unsigned char TempL2;
unsigned int ComValue2;
unsigned int T2; // 2的周期

unsigned char Compare_Flag;
unsigned char TIM1_Count;

void user_isr(); // 用户中断程序，不可删除

//===========================================================
// Funtion name：interrupt ISR
// parameters：无
// returned value：无
//===========================================================
void interrupt ISR(void)
{
#asm;			// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
#endasm;		// 系统设置不可以删除和修改
	user_isr(); // 调用用户中断函数
}
void user_isr() // 调用用户中断函数
{
	T1UIF = 1;		// 写1清零标志位
	PORTB = ~PORTB; // 翻转电平
	TIM1_Count++;
}
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 系统时钟选择为内部振荡器16MHz,分频比为1:1
	INTCON = 0;			 // 禁止所有中断

	PORTA = 0B00000000;
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA = 0B00000000; // 弱上拉的开关，0-关，1-开
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // 弱下拉的开关，0-关，1-开
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B00000000; // 输入输出设置，0-输出，1-输入,TIM2_CH1,TIM2_CH2,PA4,PA5-输出
	TRISB = 0B00000000;
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // 源电流设置最大
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // 灌电流设置最大
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // 设置对应的IO为数字IO
}
/*-------------------------------------------------
* 函数名：TIM2_INITIAL
* 功能：  初始化TIM2
* 输入：  无
* 输出：  无
--------------------------------------------------*/

void TIM2_INITIAL(void)
{
	PCKEN |= 0B00000100; // 使能TIMER2模块时钟
	CKOCON = 0B00100000; // Timer2倍频时钟占空比调节位4ns延迟
	TCKSRC = 0B00110000; // Timer2时钟源为HIRC的2倍频

	TIM2CR1 = 0B10000101; // 允许自动装载，使能计数器

	TIM2IER = 0B00000000; // 禁止所有中断

	TIM2SR1 = 0B00000000;
	TIM2SR2 = 0B00000000;

	TIM2EGR = 0B00000000;

	TIM2CCMR1 = 0B01101000; // 将通道CH1配置为输出，PWM模式1
	TIM2CCMR2 = 0B01101000; // 将通道CH2配置为输出，PWM模式1
	TIM2CCMR3 = 0B00000000;

	TIM2CCER1 = 0B00110011; // 比较1和2输出使能，低电平有效
	TIM2CCER2 = 0B00000000;

	TIM2CNTRH = 0B00000000;
	TIM2CNTRL = 0B00000000;

	TIM2ARRH = 0x03; // 自动装载高8位03H
	TIM2ARRL = 0xe8; // 自动装载低8位e8H

	TIM2CCR1H = 0x01; // 装入比较1的预装载值高8位01H
	TIM2CCR1L = 0xf4; // 装入比较1的预装载值低8位F4H

	TIM2CCR2H = 0x01; // 装入比较2的预装载值高8位01H
	TIM2CCR2L = 0xf4; // 装入比较2的预装载值低8位F4H

	TIM2CCR3H = 0B00000000;
	TIM2CCR3L = 0B00000000;
}
/*-------------------------------------------------
* 函数名：TIM1_INITIAL
* 功能：  初始化TIM1
* 输入：  无
* 输出：  无
--------------------------------------------------*/
void TIM1_INITIAL(void)
{
	PCKEN |= 0B00000010;  // 使能TIMER1模块时钟
	CKOCON = 0B00100000;  // Timer1倍频时钟占空比调节位4ns延迟
	TCKSRC |= 0B00000011; // Timer1时钟源为HIRC的2倍频

	TIM1CR1 = 0B10000101; // 允许自动装载，使能计数器

	TIM1IER = 0B00000001; // 允许更新中断

	TIM1ARRH = 0x7C; // 自动装载高8
	TIM1ARRL = 0xFF; // 自动装载低8位  1ms 进一次中断

	INTCON = 0B11000000; // 使能总中断和外设中断
}
/*-------------------------------------------------
 * 函数名：main
 * 功能：  主函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL(); // 系统初始化
	TIM2_INITIAL();
	TIM1_INITIAL();
	TempH1 = TIM2CCR1H;
	TempL1 = TIM2CCR1L;
	ComValue1 = TempH1 * 256 + TempL1;
	T1 = 2 * (TempH1 * 256 + TempL1) - 1;

	TempH2 = TIM2CCR2H;
	TempL2 = TIM2CCR2L;
	ComValue2 = TempH2 * 256 + TempL2;
	T2 = 2 * (TempH2 * 256 + TempL2) - 1;

	Compare_Flag = 0;
	unsigned char breath_Flag1 = 0;
	unsigned char breath_Flag2 = 0;

	//	TIM2CCR1H = ComValue2 / 256;
	//	TIM2CCR1L = ComValue2 % 256;

	while (1)
	{
		switch (breath_Flag1)
		{
		case 1:
			for (unsigned char i = 0; i < 1000; i++)
			{
				if (TIM1_Count > 49)
				{
					TIM1_Count = 0;
					ComValue1+=20;
					TIM2CCR1H = ComValue1 / 256;
					TIM2CCR1L = ComValue1 % 256;
					// TIM2EGR = 0B00000001;
					if (ComValue1 >= T1)
					{
						breath_Flag1 = 0;
						break;
					}
				}
			}
			break;

		case 0:
			for (unsigned char j = 0; j < 1000; j++)
			{
				if (TIM1_Count > 49)
				{
					TIM1_Count = 0;
					ComValue1-=20;
					TIM2CCR1H = ComValue1 / 256;
					TIM2CCR1L = ComValue1 % 256;
					// TIM2EGR = 0B00000001;
					if (ComValue1 <= 0)
					{
						breath_Flag1 = 1;
						break;
					}
				}
			}
			break;
		
        default:
			break;
		}

		NOP();
	}
}