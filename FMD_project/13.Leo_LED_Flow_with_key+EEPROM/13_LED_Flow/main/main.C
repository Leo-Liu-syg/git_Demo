#include "SYSCFG.h"
#include "FT64F0AX.h"

// *************宏定义***************

// *************变量定义*************
//-------------LED模块变量------------
unsigned int Time1_count = 0;
unsigned char LED_Flowflag = 0;

//-------------按键模块变量------------
unsigned int Key_press_count = 0;

// ************函数声明区域************
volatile char W_TMP @0x70;
volatile char BSR_TMP @0x71;
void user_isr();
void POWER_INITIAL(void);
void TIM1_INIT(void);

void interrupt ISR(void)
{
#asm;			// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
	NOP;		// ϵͳ���ò�����ɾ�����޸�
#endasm;		// ϵͳ���ò�����ɾ�����޸�
	user_isr(); // �����û��жϺ���
}
void user_isr() // 中断函数
{
	if (TIM1SR1 & 0x01) // 检查更新中断标志位
	{
		TIM1SR1 |= 0x01; // 写1清除T1UIF
		Time1_count++;
		// Key_press_count++;
		if (Time1_count > 2000)
		{
			Time1_count = 0;
		}
	}
}
void POWER_INITIAL(void) // PB0 PB1输入，PA全部输出
{
	OSCCON = 0B01100001;
	INTCON = 0;

	PORTA = 0B00000000;
	PORTB = 0B00000000;
	PORTC = 0B00000000;
	WPUA = 0B00000000;
	WPUB = 0B00000000;
	WPUC = 0B00000000;
	WPDA = 0B00000000;
	WPDB = 0B00000000;
	WPDC = 0B00000000;
	TRISA = 0B00000000;
	TRISB = 0B00000000;
	TRISC = 0B00000000;

	TRISA |= 0B00000000;
	TRISB |= 0B00000011;
	WPUA |= 0B00000000;
	WPUB |= 0B00000011;

	PSRC0 = 0B11111111;
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;
	PSINK0 = 0B11111111;
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;
	ANSELA = 0B00000000;
}
/*-------------------------------------------------
 * 函数名：TIM1_INIT
 * 功能：  初始化TIM1为1ms中断（用于计时）
 --------------------------------------------------*/
void TIM1_INIT(void) // 1ms进一次中断
{
	PCKEN |= 0B00000010;  // 使能Timer1模块时钟
	CKOCON = 0B00100000;  // 时钟输出配置（不影响中断，保持原配置）
	TCKSRC |= 0B00000011; // Timer1时钟源=2x HIRC（32MHz）

	// 配置预分频器（显式设置1分频，确保计数时钟=32MHz）
	TIM1PSCRH = 0x00;
	TIM1PSCRL = 0x00; // 预分频比=0x0000+1=1

	TIM1CR1 = 0B10000101; // 使能自动重载、边沿对齐、向上计数、使能计数器
	TIM1IER = 0B00000001; // 使能更新中断
	TIM1ARRH = 0x7C;	  // 自动重载值高8位（0x7CFF=31999）
	TIM1ARRL = 0xFF;	  // 自动重载值低8位
	INTCON = 0B11000000;  // 使能全局中断和外设中断
}

main()
{
	POWER_INITIAL();
	TIM1_INIT();

	while (1)
	{
		// for (unsigned char i = 0; i < 4; i++)
		// {
		// 	if (Time1_count == 500)
		// 	{
		// 		PORTA = PORTA & (~(0B00000001 << i));
		// 	}
		// 	if (Time1_count == 1000)
		// 	{
		// 		PORTA = PORTA & (~(0B00000001 << i));
		// 	}
		// 	if (Time1_count == 1500)
		// 	{
		// 		PORTA = PORTA & (~(0B00000001 << i));
		// 	}
		// 	if (Time1_count == 2000)
		// 	{
		// 		PORTA = PORTA & (~(0B00000001 << i));
		// 	}
		// }
		if (Time1_count < 500 && Time1_count >=0)
		{
			PORTA=0B11101111;
		}
		else if (Time1_count < 1000 && Time1_count >=500)
		{
			PORTA=0B11011111;
		}
		else if (Time1_count < 1500 && Time1_count >=1000)
		{
			PORTA=0B10111111;
		}
		else if (Time1_count < 2000 && Time1_count >=1500)
		{
			PORTA=0B01111111;
		}
	}
}
