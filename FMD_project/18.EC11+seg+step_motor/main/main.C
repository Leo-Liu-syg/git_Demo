// Project: project.prj
//  Device: FT61F0AX
//  Memory: Flash 10KX14b, EEPROM 128X8b, SRAM 1KX8b
//  Author:
// Company:
// Version:
// PA0/1 分别接EC11旋转编码器BB/GA，PB3/4接数码管
//===========================================================
//===========================================================
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "TDelay.h"
#include "TM1650_IIC.h"
#include "EC11.h"
//===========================================================

//***********************宏定义****************************
#define uint unsigned int
#define ulong unsigned long
#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8�����ȣ�7����ʾ��������ʾʹ�ܣ��ֲ�����ˣ�
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL PB3
#define SDA PB4

#define EC11_A PA7
#define EC11_B PA6

#define Key_A PA0
#define Key_B PA1

#define LED PA1

// 数码管变量定义
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

unsigned int Number_Sum = 0;
unsigned int Number_Sum_Old = 0;
unsigned int Number_Ge = 0;
unsigned int Number_Shi = 0;
unsigned int Number_Bai = 0;
unsigned int Number_Qian = 0;
unsigned char Seg_flag = 0;

// LED_Breath
unsigned char LED_Count = 0;
unsigned char LED_PWM_Mid = 0;
unsigned char Breath_Flag1 = 0;

// 按键扫描消抖
unsigned char Motor_flag = 0;
unsigned char key_statue_buffer = 0;
unsigned char key_statue = 0;
unsigned char key_count = 0;
unsigned char PWM_count = 0;

// 步进电机 IO: 0 1 4 5
unsigned char P_Motor[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}; // 正转
unsigned char N_Motor[4][4] = {{0, 0, 0, 1}, {0, 0, 1, 0}, {0, 1, 0, 0}, {1, 0, 0, 0}}; // 反转
unsigned char i_P_Current;
unsigned char i_P_Old;
unsigned char Motor_Tick = 0;
unsigned int Motor_T = 0;

// 编码器相关变量
unsigned char EC11_State_Old = 0;	 // 编码器上一次状态（A/B相组合）
unsigned char EC11_Debounce_Cnt = 0; // 编码器防抖计数

// Timer1
unsigned int Debounce_count = 0;
unsigned int Timer1_count1 = 0;
unsigned int Timer1_count2 = 0;
unsigned int Timer1_count3 = 0;
unsigned char flag_1ms = 0;

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
	if (T1UIF)
	{
		T1UIF = 1;
		Timer1_count1++;
		Timer1_count2++;
		Timer1_count3++;
		if (Timer1_count1 >= 40) // 2000us
		{
			flag_1ms = 1;
			Timer1_count1 = 0;
		}
		if (Timer1_count2 >= 85 + Motor_T) // 2.5ms+150us*(0--99)
		{
			Motor_flag = 1;
			Timer1_count2 = 0;
		}
		if (Timer1_count3 >= 3000) // 150ms
		{
			Seg_flag = 1;
			Timer1_count3 = 0;
		}
	}
}

/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能： 	 上电系统初始化
 * 输入：	 无
 * 输出： 	 无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 系统时钟选择为内部振荡器8MHz,分频比为1:1

	PCKEN |= 0B00000010; // 使能定时器1

	INTCON = 0; // 禁止所有中断

	PORTA = 0B00000000;
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA |= 0B11000000; // EC11_A/EC11_B（编码器）使能弱上拉 // 弱上拉的开关，0-关，1-开
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // 弱下拉的开关，0-关，1-开
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B11000000; // 输入输出设置，0-输出，1-输入 修改为EC11_A、EC11_B为输入
	TRISB = 0B00000010; // PB1输入
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // 源电流设置最大
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // 灌电流设置最大
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // 设置对应的IO为数字IO
}

void TIM1_INITIAL(void)
{
	PCKEN |= 0B00000010;  // ???TIMER1??????
	CKOCON = 0B00100000;  // Timer1?????????????λ4ns???
	TCKSRC |= 0B00000011; // Timer1?????HIRC??2???

	TIM1CR1 = 0B10000101; // ???????????????????

	TIM1IER = 0B00000001; // ?????????ж?

	TIM1ARRH = 0x06; // ???????8
	TIM1ARRL = 0x40; // ???????8λ  50us ??????ж?

	INTCON = 0B11000000; // ??????ж???????ж?
}

// void Fan_Start(void)
// {
// 	// 风扇转速
// 	Fan_Count++;
// 	if (Fan_Count > Number_Sum)
// 	{
// 		PA0 = 1; // 风扇转；
// 	}
// 	else if (Fan_Count <= Number_Sum)
// 	{
// 		PA0 = 0; // 风扇停
// 	}

// 	if (Fan_Count > 99)
// 	{
// 		Fan_Count = 0;
// 	}
// }
// void LED_Breath(void)
// {
// 	if (Breath_Flag1 == 1)
// 	{
// 		PWM_count++;
// 		if (PWM_count >= 20)
// 		{
// 			if (LED_PWM_Mid < 99)
// 				LED_PWM_Mid++;
// 			PWM_count = 0;
// 		}
// 	}

// 	else if (Breath_Flag1 == 2)
// 	{
// 		PWM_count++;
// 		if (PWM_count >= 20)
// 		{
// 			if (LED_PWM_Mid > 0)
// 				LED_PWM_Mid--;
// 			PWM_count = 0;
// 		}
// 	}
// 	// 亮灭的核心
// 	LED_Count++;
// 	if (LED_Count > LED_PWM_Mid)
// 	{
// 		LED = 1; // 灯灭
// 	}
// 	else if (LED_Count <= LED_PWM_Mid)
// 	{
// 		LED = 0; // 灯亮
// 	}
// 	if (LED_Count > 99)
// 	{
// 		LED_Count = 0;
// 	}
// }
void Key_control_debounce(void) // 按键消抖
{
	if (Key_A == 0 && LED_PWM_Mid <= 99 && Key_B == 1)
	{
		key_statue_buffer = 1;
	}
	else if (Key_B == 0 && LED_PWM_Mid > 0 && Key_A == 1)
	{
		key_statue_buffer = 2;
	}
	else if (Key_A == 0 && Key_B == 0)
	{
		key_statue_buffer = 0;
	}
	else if (Key_A == 1 && Key_B == 1)
	{
		key_statue_buffer = 0;
	}

	if (key_statue != key_statue_buffer)
	{
		key_count++;
		if (key_count >= 10)
		{
			key_count = 0;
			key_statue = key_statue_buffer;
			// 根据按键状态设置呼吸灯模式
			switch (key_statue)
			{
			case 0: // 两键都松开 - 停止呼吸，保持当前亮度
				Breath_Flag1 = 0;
				break;
			case 1: // Key_A按下 - 开始渐亮
				Breath_Flag1 = 1;
				break;
			case 2: // Key_B按下 - 开始渐暗
				Breath_Flag1 = 2;
				break;
			case 3: // 两键都按下 - 快速回到0亮度
				Breath_Flag1 = 0;
				LED_PWM_Mid = 0;
				break;
			}
		}
	}
	else
	{
		key_count = 0;
	}
}
void Seg_Display(void)
{
	// 数据处理，显示在数码管

	Number_Ge = Number_Sum % 10;
	Number_Shi = Number_Sum / 10;
	if (Number_Sum >= 0 && Number_Sum < 10)
	{
		TM1650_Set(led_place[0], seg_code[Number_Ge]);
		TM1650_Set(led_place[1], 0);
		TM1650_Set(led_place[2], 0);
		TM1650_Set(led_place[3], 0);
	}
	else if (Number_Sum >= 10 && Number_Sum < 100)
	{
		TM1650_Set(led_place[0], seg_code[Number_Shi]);
		TM1650_Set(led_place[1], seg_code[Number_Ge]);
		TM1650_Set(led_place[2], 0);
		TM1650_Set(led_place[3], 0);
	}
}

void main(void)
{
	POWER_INITIAL(); // �0�3�0�1�1�7�1�7�0�3�1�7�1�7
	TIM1_INITIAL();
	TM1650_Init();
	TM1650_Set(led_place[0], seg_code[9]); // 能显示，测试
	EC11_State_Old = EC11_Read_State();
	LED = 1;
	while (1)
	{

		Motor_T = Number_Sum * 6;
		if (flag_1ms == 1) // 2ms进来执行一次
		{
			// 旋钮控制数据
			EC11_Process();
			flag_1ms = 0;
		}
		if (Motor_flag == 1) // 进来的速度由Number_Sum决定
		{
			// Key_control_debounce();
			// LED_Breath();
			// 步进电机

			PA0 = P_Motor[i_P_Current][0];

			PA1 = P_Motor[i_P_Current][1];

			PA4 = P_Motor[i_P_Current][2];

			PA5 = P_Motor[i_P_Current][3];
			if (i_P_Current < 4)
			{
				i_P_Current++;
			}
			else
			{
				i_P_Current = 0;
			}
			Motor_flag = 0;
		}

		if (Seg_flag == 1) // 150ms
		{
			if (Number_Sum_Old != Number_Sum)
			{
				Seg_Display(); // 里面有Delay
				Number_Sum_Old = Number_Sum;
			}
			Seg_flag = 0;
		}
	}
}
//===========================================================
