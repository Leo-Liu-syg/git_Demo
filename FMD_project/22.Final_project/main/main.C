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
#include "LED.h"
#include "TDelay.h"
#include "TM1650_IIC_1.h"
#include "TM1650_IIC_2.h"

#define u8 unsigned char
#define u16 unsigned int
#define LED_Left PA1
#define LED_Mid PA3
#define LED_Right PA4
#define Buzz PA5

// 变量定义
volatile u16 Time_1ms_count = 0;
volatile u8 Time_flag = 0;
volatile u8 Seg_1_flag = 0;
volatile u8 Seg_2_flag = 0;

// 数码管变量定义
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

// 数码管1
float Number_Sum_1 = 0.0f;
float Number_Sum_1_Old = 0.0f;
float Number_Sum_1_Abs = 0.0f; // 用于处理负数
int Number_Ge_1 = 0;
int Number_Shi_1 = 0;
int Number_Bai_1 = 0;
int Number_Qian_1 = 0;
char Number_Dec_1 = 0;

// 数码管2
float Number_Sum_2 = 0.0f;
float Number_Sum_2_Old = 0.0f;
float Number_Sum_2_Abs = 0.0f; // 用于处理负数
int Number_Ge_2 = 0;
int Number_Shi_2 = 0;
int Number_Bai_2 = 0;
int Number_Qian_2 = 0;
char Number_Dec_2 = 0;

//===========================================================
// Variable definition
volatile char W_TMP @0x70;	 // ϵͳռ�ò�����ɾ�����޸�
volatile char BSR_TMP @0x71; // ϵͳռ�ò�����ɾ�����޸�
void user_isr();			 // �û��жϳ��򣬲���ɾ��
//===========================================================

//===========================================================
// Function name��interrupt ISR
// parameters����
// returned value����
//===========================================================
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
#endasm;		// ϵͳ���ò�����ɾ�����޸�
	user_isr(); // �û��жϺ���
}
void user_isr() // �û��жϺ���
{
	if (T1UIE && T1UIF)
	{
		T1UIF = 1;

		if (Time_1ms_count > 999)
		{
			Time_flag = 1;
			Time_1ms_count = 0;
		}
		Time_1ms_count++;
	}
}

void TIM1_Init(void) // 1ms
{
	PCKEN |= 0B00000010;
	CKOCON = 0B00100000;
	TCKSRC = 0B00000011;

	TIM1CR1 = 0B10000101;
	TIM1IER = 0B00000001;

	TIM1ARRH = 0x7C; // �Զ�װ�ظ�8λH
	TIM1ARRL = 0x90; // �Զ�װ�ص�8λH

	INTCON = 0B11000000;
}

// 控制输入输出/若上拉的函数，根据实际情况调整，就不解耦了
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 8Mhz
	INTCON |= 0x80;

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

	PSRC0 = 0B00000000; // 源电流开到最小
	PSRC1 = 0B00000000;
	PSRC2 = 0B00000000;

	PSINK0 = 0B00000000; // 灌电流开到最小
	PSINK1 = 0B00000000;
	PSINK2 = 0B00000000;

	ANSELA = 0B00000000;
}
void Seg1_Display(void) // 正负数（最大99）
{
	// 数据处理，显示在数码管

	if (Number_Sum_1 >= 0)
	{
		Number_Ge_1 = ((int)Number_Sum_1) % 10;
		Number_Shi_1 = ((int)Number_Sum_1) / 10;
		Number_Dec_1 = (int)(Number_Sum_1 * 10) % 10;
		if (Number_Sum_1 >= 0 && Number_Sum_1 < 10)
		{
			TM1650_1_Set(led_place[0], 0);
			TM1650_1_Set(led_place[1], seg_code[Number_Ge_1] + 0x80);
			TM1650_1_Set(led_place[2], seg_code[Number_Dec_1]);
			TM1650_1_Set(led_place[3], 0b00111001);
		}
		else if (Number_Sum_1 >= 10 && Number_Sum_1 < 100)
		{
			TM1650_1_Set(led_place[0], seg_code[Number_Shi_1]);
			TM1650_1_Set(led_place[1], seg_code[Number_Ge_1] + 0x80);
			TM1650_1_Set(led_place[2], seg_code[Number_Dec_1]);
			TM1650_1_Set(led_place[3], 0b00111001);
		}
	}
	if (Number_Sum_1 < 0)
	{
		Number_Sum_1_Abs = -Number_Sum_1; // 转为正数
		Number_Ge_1 = ((int)Number_Sum_1_Abs) % 10;
		Number_Shi_1 = ((int)Number_Sum_1_Abs) / 10;
		Number_Dec_1 = (int)(Number_Sum_1_Abs * 10) % 10;
		if (Number_Sum_1_Abs >= 0 && Number_Sum_1_Abs < 10)
		{
			TM1650_1_Set(led_place[0], 0x40); // 负号
			TM1650_1_Set(led_place[1], seg_code[Number_Ge_1] + 0x80);
			TM1650_1_Set(led_place[2], seg_code[Number_Dec_1]);
			TM1650_1_Set(led_place[3], 0b00111001);
		}
		else if (Number_Sum_1_Abs >= 10 && Number_Sum_1_Abs < 100)
		{
			TM1650_1_Set(led_place[0], 0x40);
			TM1650_1_Set(led_place[1], seg_code[Number_Shi_1]);
			TM1650_1_Set(led_place[2], seg_code[Number_Ge_1]);
			TM1650_1_Set(led_place[3], 0b00111001);
		}
	}
}

void Seg2_Display(void) // 正负数（最大99）
{
	// 数据处理，显示在数码管

	if (Number_Sum_2 >= 0)
	{
		Number_Ge_2 = ((int)Number_Sum_2) % 10;
		Number_Shi_2 = ((int)Number_Sum_2) / 10;
		Number_Dec_2 = ((int)(Number_Sum_2 * 10)) % 10;
		if (Number_Sum_2 >= 0 && Number_Sum_2 < 10)
		{
			TM1650_2_Set(led_place[0], 0);
			TM1650_2_Set(led_place[1], seg_code[Number_Ge_2] + 0x80);
			TM1650_2_Set(led_place[2], seg_code[Number_Dec_2]);
			TM1650_2_Set(led_place[3], 0b01110110);
		}
		else if (Number_Sum_2 >= 10 && Number_Sum_2 < 100)
		{
			TM1650_2_Set(led_place[0], seg_code[Number_Shi_2]);
			TM1650_2_Set(led_place[1], seg_code[Number_Ge_2] + 0x80);
			TM1650_2_Set(led_place[2], seg_code[Number_Dec_2]);
			TM1650_2_Set(led_place[3], 0b01110110);
		}
	}
	if (Number_Sum_2 < 0)
	{
		Number_Sum_2_Abs = -Number_Sum_2; // 转为正数
		Number_Ge_2 = ((int)Number_Sum_2_Abs) % 10;
		Number_Shi_2 = ((int)Number_Sum_2_Abs) / 10;
		Number_Dec_2 = ((int)(Number_Sum_2_Abs * 10)) % 10;
		if (Number_Sum_2_Abs >= 0 && Number_Sum_2_Abs < 10)
		{
			TM1650_2_Set(led_place[0], 0x40); // 负号
			TM1650_2_Set(led_place[1], seg_code[Number_Ge_2] + 0x80);
			TM1650_2_Set(led_place[2], seg_code[Number_Dec_2]);
			TM1650_2_Set(led_place[3], 0b01110110);
		}
		else if (Number_Sum_2_Abs >= 10 && Number_Sum_2_Abs < 100)
		{
			TM1650_2_Set(led_place[0], 0x40);
			TM1650_2_Set(led_place[1], seg_code[Number_Shi_2]);
			TM1650_2_Set(led_place[2], seg_code[Number_Ge_2]);
			TM1650_2_Set(led_place[3], 0b01110110);
		}
	}
}

void main(void)
{
	POWER_INITIAL();
	TIM1_Init();
	TM1650_1_Init();
	TM1650_2_Init();
	Buzz = 0;
	LED_Buzz_turn_on();
	TM1650_1_Set(led_place[1], seg_code[1]);
	TM1650_2_Set(led_place[2], seg_code[2]);
	while (1)
	{
		if (Time_flag == 1)
		{
			LED_Buzz_turn_off();
			Buzz = 1;
			Time_flag = 0;
		}
	}
}
//===========================================================
