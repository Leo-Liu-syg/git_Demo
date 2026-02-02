// Project: main.prj
//  Device: FT61F0AX
//  Memory: Flash 10KX14b, EEPROM 128X8b, SRAM 1KX8b
//  Author:
// Company:
// Version:
//    Date:
//===========================================================
//===========================================================
#include "SYSCFG.h"
#include "FT61F0AX.h"
#include "hw_timer1.h"
//===========================================================
// Variable definition
volatile char W_TMP @0x70;	 // 系统占用不可以删除和修改
volatile char BSR_TMP @0x71; // 系统占用不可以删除和修改
void user_isr();			 // 用户中断程序，不可删除
//===========================================================

//===========================================================
// Function name：interrupt ISR
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
	user_isr(); // 用户中断函数
}
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 系统时钟选择为内部振荡器8MHz, 分频比为1:1

	INTCON = 0; // 禁止所有中断

	PORTA = 0B00000000;
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA = 0B00000000; // 弱上拉的开关，0-关，1-开
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // 弱下拉的开关，0-关，1-开
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B00010000; // PA输入输出，0-输出，1-输入 PA4-输入,PA5输出
	TRISB = 0B00000000; // PB输入输出，0-输出，1-输入
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // 源电流设置最大
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // 灌电流设置最大
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // 设置对应的IO为数字IO
}

unsigned char flag_1ms = 0;
unsigned int count_1s = 0;

void user_isr() // 用户中断函数
{
	if (T1UIF)
	{
		T1UIF = 0;
		flag_1ms = 1;
	}
}
//===========================================================
// Function name：main
// parameters：无
// returned value：无
//===========================================================
main()
{
	POWER_INITIAL();
	HW_TIMER1_Init();

	while (1)
	{
		if (flag_1ms == 1)
		{
			flag_1ms = 0;
			count_1s += 1;
			if(count_1s == 1000)
			{
				count_1s = 0;
				PORTB = ~PORTB;
			}
		}
	}
}
//===========================================================
