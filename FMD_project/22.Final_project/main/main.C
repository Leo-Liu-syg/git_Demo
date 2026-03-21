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
#include "IIC_SHT.h"
#include "Seg_Control_Package.h"
#include "ADC.h"
#include "EEPROM.h"

#define u8 unsigned char
#define u16 unsigned int
#define LED_Left PA0
#define LED_Mid PA1
#define LED_Right PA3
#define Buzz PB5

// 摄氏度/华氏度
#define S 0
#define H 1

// 中断内变量定义
volatile u16 Start_Count = 0;

volatile u8 Start_Flag = 0;

volatile u16 volatile_steady_count = 0;
volatile u8 volatile_steady_Flag = 0;

volatile u16 LED_Flash_Count = 0;
volatile u8 LED_Flash_Flag = 0;
volatile u8 LED_Flash_Start_Flag = 0;

volatile u8 Warm_ready = 0;

volatile u16 Count_10s = 0;
volatile u8 Flag_10s = 0;

volatile u8 Seg_2_Fresh_Flag = 0;
volatile u16 Seg_2_Fresh_Count = 0;

// 数码管变量定义
extern unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
extern unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};
// 数码管1
extern volatile float Number_Sum_1 = 0.0f;
extern float Number_Sum_1_Old = 0.0f;
extern float Number_Sum_1_Abs = 0.0f; // 用于处理负数
extern int Number_Ge_1 = 0;
extern int Number_Shi_1 = 0;
extern int Number_Bai_1 = 0;
extern int Number_Qian_1 = 0;
extern char Number_Dec_1 = 0;
// 数码管2
extern volatile float Number_Sum_2 = 0.0f;
extern float Number_Sum_2_Old = 0.0f;
extern float Number_Sum_2_Abs = 0.0f; // 用于处理负数
extern int Number_Ge_2 = 0;
extern int Number_Shi_2 = 0;
extern int Number_Bai_2 = 0;
extern int Number_Qian_2 = 0;
extern char Number_Dec_2 = 0;

// 温湿度
extern unsigned int sht_data_buf[6] = {0};
extern float sht_temperature = 0.0f;
extern float sht_humidity = 0.0f;
extern unsigned int temp_raw = 0;
extern unsigned int humi_raw = 0;
extern volatile unsigned char MODE_SorH = 0;

// voltage
unsigned int adcData = 0;
unsigned long theVoltage = 0;
unsigned long temp_num_2 = 0;
u8 V_marked = 0;
u8 V_current = 0;

//-------------EEPROM模块变量----------
unsigned char EEPROM_H = 0;
unsigned char EEPROM_L = 0;
//===========================================================
// Variable definition
volatile char W_TMP @0x70;
volatile char BSR_TMP @0x71;
void user_isr();
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
		Start_Count++;
		LED_Flash_Count++;
		Seg_2_Fresh_Count++;
		volatile_steady_count++;
		Count_10s++;
		if (LED_Flash_Count > 500)
		{
			LED_Flash_Flag = 1;
			LED_Flash_Count = 0;
		}
		if (Seg_2_Fresh_Count > 500)
		{
			Seg_2_Fresh_Flag = 1;
			Seg_2_Fresh_Count = 0;
		}
		if (Start_Count > 999)
		{
			Start_Flag = 1;
			Start_Count = 0;
		}
		if (volatile_steady_count > 500)
		{
			volatile_steady_count = 0;
		}
		if (Count_10s >= 10000)
		{
			Count_10s = 0;
			Flag_10s = 1;
		}
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
	PCKEN |= 0B00000001; // AD模块时钟使能
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
	TRISB = 0B00000010; // PB1输入
	TRISC = 0B00000000;

	PSRC0 = 0B00000000; // 源电流开到最小
	PSRC1 = 0B00000000;
	PSRC2 = 0B00000000;

	PSINK0 = 0B00000000; // 灌电流开到最小
	PSINK1 = 0B00000000;
	PSINK2 = 0B00000000;

	ANSELA = 0B00000000;
}

void main(void)
{
	POWER_INITIAL();
	TIM1_Init();
	TM1650_1_Init();
	TM1650_2_Init();
	ADC_INITIAL();
	Buzz = 0;
	LED_turn_on();
	Seg1_Init_Ready();
	Seg2_Init_Ready();
	Number_Sum_2 = 99;
	while (1)
	{
		switch (Warm_ready) // 预热结束后进入状态机
		{
		case 0:
			if (LED_Flash_Flag == 1 && LED_Flash_Start_Flag == 1) // 预热灯
			{
				LED_All_Shinning();
				LED_Flash_Flag = 0;
			}
			if (Start_Flag == 1)//1s执行一次
			{
				LED_Flash_Start_Flag = 1;
				Buzz = 1;
				SHT_process();
				SHT_Data_process();
				Number_Sum_1 = sht_temperature;
				Seg1_Display();
				if (Number_Sum_2 > 0 && Warm_ready == 0)
				{
					Number_Sum_2--;
					Seg2_Display();
				}

				if (Number_Sum_2 <= 90 && Warm_ready == 0) // 倒计时结束
				{
					Warm_ready = 1;
					break;
				}
				Start_Flag = 0;
			}
			break;
		case 1:
			if (LED_Left == 1)
			{
				LED_turn_on();
			}
			adcData = GET_ADC_DATA(0);
			// 计算电压（全程整数运算，避免浮点）
			theVoltage = ((unsigned long)adcData * 2UL * 1000UL) / 4096UL;
			// 保留 Number_Sum_2 为 float 类型
			Number_Sum_2 = (float)theVoltage;
			// 关键：先转成无符号长整型临时变量，用它来做取模运算
			unsigned long temp_num_2 = (unsigned long)Number_Sum_2;
			// 用临时变量 temp_num_2 做数位提取（注意变量名要一致！）
			Number_Ge_2 = temp_num_2 / 1000;
			Number_Shi_2 = (temp_num_2 / 100) % 10;
			Number_Bai_2 = (temp_num_2 / 10) % 10;
			Number_Qian_2 = temp_num_2 % 10;
			if (Seg_2_Fresh_Flag == 1) // 0.5s更新一次
			{
				TM1650_2_Set(led_place[0], seg_code[Number_Ge_2] + 0x80);
				TM1650_2_Set(led_place[1], seg_code[Number_Shi_2]);
				TM1650_2_Set(led_place[2], seg_code[Number_Bai_2]);
				TM1650_2_Set(led_place[3], seg_code[Number_Qian_2]);
				Seg_2_Fresh_Flag = 0;
			}
			if (temp_num_2 >= 1000 && temp_num_2 < 2000)
			{
				V_marked = temp_num_2;
				Warm_ready = 2;
				break;
			}
			break;
		case 2:
			LED_mid_right_on();
			if (Flag_10s == 1) // 10s进来一次
			{
				LED_Left_on();
				V_current = temp_num_2;
				if (V_current > V_marked)
				{
					EEPROM_H = (V_current - V_marked) / 256;
					EEPROM_L = (V_current - V_marked) % 256;
					EEPROM_Write(0x00, EEPROM_H);
					EEPROM_Write(0x01, EEPROM_L);
				}
				Flag_10s = 0;
				LED_Left_off();
				break;
			}
			break;
		}
	}
}
//===========================================================
