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
#include "Key_Scan_Non_Block.h"

#define u8 unsigned char
#define u16 unsigned int
#define LED_Left PA0
#define LED_Mid PA1
#define LED_Right PA3
#define Buzz PB5
#define Key_1 PA4
#define Key_2 PA5

// 摄氏度/华氏度
#define S 0
#define H 1

// 中断内变量定义
volatile u16 Start_Count = 0;
u8 Start_Flag = 0;

volatile u16 volatile_steady_count = 0;
u8 volatile_steady_Flag = 0;

volatile u16 LED_Flash_Count = 0;
u8 LED_Flash_Flag = 0;
u8 LED_Flash_Start_Flag = 0;

volatile u16 Count_10s = 0;
u8 Flag_10s = 0;

u8 Seg_2_Fresh_Flag = 0;
volatile u16 Seg_2_Fresh_Count = 0;

volatile u16 Count_30s = 0;
u8 Flag_30s = 0;

volatile u8 Count_200ms = 0;
u8 Flag_200ms = 0;

u8 Flag_1ms = 0;

//------------------------变量相关定义-----------------------------
// 数码管数组
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

// 按键
u8 alarm_Flag;		   // 报警标志：1=报警中
u8 calibrated_Flag;	   // 已标定标志：1=已标定
u8 power_on_key2_flag; // 上电时检测到Key2按住（main里初始化一次）
extern u8 key_state = 0;

// 温湿度
extern unsigned int sht_data_buf[6] = {0};
extern float sht_temperature = 0.0f;
extern float sht_humidity = 0.0f;
extern unsigned int temp_raw = 0;
extern unsigned int humi_raw = 0;
extern volatile unsigned char MODE_SorH = 0;

// voltage
extern volatile unsigned int adcData = 0;
extern volatile unsigned long theVoltage = 0;
extern volatile unsigned long temp_num_2 = 0;
extern volatile unsigned long V_marked = 0;
extern volatile unsigned long V_current = 0;
volatile float delta = 0;

// 工作状态
volatile u8 Work_Mode = 0;
volatile u8 Calibration = 0;
volatile u8 standard_work = 0;
u8 power_on_key2_flag = 0;
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
		Count_30s++;
		Count_200ms++;
		Flag_1ms = 1;
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
		if (Count_30s >= 30000)
		{
			Count_30s = 0;
			Flag_30s = 1;
		}
		if (Count_200ms >= 200)
		{
			Count_200ms = 0;
			Flag_200ms = 1;
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

	WPUA = 0B00110000; // 按键若上拉
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000;
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B00110000; // 按键输入
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
	Seg1_Init_Ready();
	Seg2_Init_Ready();

	// 如果以前有数据，则进入正常工作状态
	V_marked = 0;
	EEPROM_H = EEPROM_Read(0x00);
	EEPROM_L = EEPROM_Read(0x01);
	V_marked = (unsigned long)(EEPROM_H * 256 + EEPROM_L);
	if (V_marked != 0xFFFFFFFF)
	{
		Work_Mode = 1;
		//V_marked = 0;
	}
	else
	{
		V_marked = 0;
	}

	// 是否按住sw2,按住则改为标定流程
	power_on_key2_flag = 1;
	NOP();
	if (Key_2 == 0)
	{
		key_state = 2;
	}
	Key_Scan_NonBlock();
	NOP();
	power_on_key2_flag = 0;

	// 重置预热倒计时
	Number_Sum_2 = 99;

	// 上电蜂鸣器响+灯亮1s
	LED_turn_on();
	Start_Count = 0;
	Buzz = 0;

	while (1)
	{
		switch (Work_Mode)
		{
		case 0:
			switch (Calibration)
			{
			case 0:													  // 预热状态
				if (LED_Flash_Flag == 1 && LED_Flash_Start_Flag == 1) // 预热灯```
				{
					LED_All_Shinning();
					LED_Flash_Flag = 0;
				}
				if (Start_Flag == 1) // 1s执行一次
				{
					LED_Flash_Start_Flag = 1;
					Buzz = 1;
					SHT_process();
					SHT_Data_process();
					Number_Sum_1 = sht_temperature;
					Seg1_Display();
					if (Number_Sum_2 > 0 && Calibration == 0)
					{
						Number_Sum_2--;
						Seg2_Display();
					}
					if (Number_Sum_2 <= 90 && Calibration == 0) // 倒计时结束
					{
						Calibration = 1;
						break;
					}
					Start_Flag = 0;
				}
				break;
			case 1: // 等待电压到达1~2v
				if (LED_Left == 1)
				{
					LED_turn_on();
				}
				Seg2_ADC_Data_process();
				if (Seg_2_Fresh_Flag == 1) // 0.5s更新一次
				{
					Seg2_Show_Voltage();
					Seg_2_Fresh_Flag = 0;
				}
				if (temp_num_2 >= 1000 && temp_num_2 < 2000)
				{
					V_marked = temp_num_2;
					Calibration = 2;
					break;
				}
				else if (temp_num_2 > 2000) // 错误溢出，蜂鸣器报警提示
				{
					Buzz = 0;
					Calibration = 0;
					break;
				}
				break;
			case 2: // 记录当前电压和标定电压差值
				LED_mid_right_on();
				Seg2_ADC_Data_process();
				if (Seg_2_Fresh_Flag == 1) // 0.5s更新一次
				{
					Seg2_Show_Voltage();
					Seg_2_Fresh_Flag = 0;
				}
				if (Flag_10s == 1) // 10s进来一次,
				{
					V_current = temp_num_2;
					if (V_current > V_marked)
					{
						EEPROM_H = (V_current - V_marked) / 256;
						EEPROM_L = (V_current - V_marked) % 256;
						EEPROM_Write(0x00, EEPROM_H);
						EEPROM_Write(0x01, EEPROM_L);
						power_on_key2_flag = 0;
						Work_Mode = 1;
						Calibration = 0;
						break;
					}
					Flag_10s = 0;
					LED_Left_shinning();
					break;
				}
				break;
			default:
				break;
			}
			break;
			// 正常工作的代码
		case 1:
			switch (standard_work)
			{
			case 0: // 上电
				Buzz = 0;
				LED_turn_on();
				if (Start_Flag == 1) // 1s执行一次后跳出
				{
					LED_Flash_Start_Flag = 1;
					Buzz = 1;
					Number_Sum_2 = 99;
					standard_work = 1; // 倒计时结束,状态机进入下一步
					Start_Flag = 0;
					break;
				}
				break;
			case 1: // 2.1预热状态
				// 数码管1显示温度
				if (Start_Flag == 1) // 1s执行一次
				{
					SHT_process();
					SHT_Data_process();
					Number_Sum_1 = sht_temperature;
					Seg1_Display();
					if (Number_Sum_2 > 0)
					{
						Number_Sum_2--;
						Seg2_Display();
					}
					if (Number_Sum_2 <= 90) // 倒计时结束,状态机进入下一步
					{
						Seg2_ADC_Data_process();
						Seg2_Show_Voltage();

						//记录当前电压值V0
						EEPROM_H = temp_num_2 / 256;
						EEPROM_L = temp_num_2 % 256;
						EEPROM_Write(0x10, EEPROM_H);
						EEPROM_Write(0x11, EEPROM_L);
						standard_work = 2;
						break;
					}
					Start_Flag = 0;
				}
				if (LED_Flash_Flag == 1 && LED_Flash_Start_Flag == 1) // 预热灯0.5s闪烁
				{
					LED_Mid = ~LED_Mid;
					LED_Flash_Flag = 0;
				}
				break;
			case 2:						 // 2.3正常监测
				if (LED_Flash_Flag == 1) // 0.5s
				{
					// seg1显示温度
					SHT_process();
					SHT_Data_process();
					Number_Sum_1 = sht_temperature;
					Seg1_Display();
					// seg2显示电压
					Seg2_ADC_Data_process();
					Seg2_Show_Voltage();
					V_current = temp_num_2;
					LED_Mid = 0; // 绿灯常亮
					// 先算差值（强转成 float，避免溢出）
					delta = (float)V_current - (float)V_marked;
					// 再判断
					if (delta > 0.0f && (float)(V_current - V_marked) <= 0.6f * (float)V_marked)
					{
						// 30s闪烁
						if (Flag_30s == 1)
						{
							LED_Right = 0;
							Flag_30s = 0;
						}
						if (LED_Right == 0 && Count_30s >= 100)
						{
							LED_Right = ~LED_Right;
							Count_30s = 0;
						}
					}
					else if ((float)(V_current - V_marked) > 0.6f * (float)V_marked)
					{
						Buzz = 0;
						LED_turn_on();
						standard_work = 3;
						break;
					}
					LED_Flash_Flag = 0;
				}
				break;
			case 3:						 // 报警状态
				if (LED_Flash_Flag == 1) // 0.5s
				{
					// seg1显示温度
					SHT_process();
					SHT_Data_process();
					Number_Sum_1 = sht_temperature;
					Seg1_Display();
					// seg2显示电压
					Seg2_ADC_Data_process();
					Seg2_Show_Voltage();
					V_current = temp_num_2;
					LED_Mid = 0; // 绿灯常亮
					// 先算差值（强转成 float，避免溢出）
					delta = (float)V_current - (float)V_marked;
					LED_Flash_Flag = 0;
				}
				if (delta > 0.0f && (float)(V_current - V_marked) <= 0.55f * (float)V_marked)
				{
					Buzz = 1;
					standard_work = 2;
					break;
				}
				if (Flag_200ms == 1)
				{
					LED_Left = 0;
					Flag_200ms = 0;
				}
				if (Count_200ms >= 50 && LED_Left == 0)
				{
					LED_Left = ~LED_Left;
					Count_200ms = 0;
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		// 按键扫描
		if (Flag_1ms == 1)
		{
			Key_Scan_NonBlock();
			Flag_1ms = 0;
		}
	}
}
//===========================================================
