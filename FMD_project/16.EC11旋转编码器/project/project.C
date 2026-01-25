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
//===========================================================

//***********************宏定义****************************
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long
#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8�����ȣ�7����ʾ��������ʾʹ�ܣ��ֲ�����ˣ�
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL PB3
#define SDA PB4

#define EC11_A PA7
#define EC11_B PA6

// 数码管变量定义
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

unsigned int Number_Sum = 0;
unsigned int Number_Ge = 0;
unsigned int Number_Shi = 0;
unsigned int Number_Bai = 0;
unsigned int Number_Qian = 0;

unsigned char i = 0;



// 编码器相关变量
uchar EC11_State_Old = 0;	 // 编码器上一次状态（A/B相组合）
uchar EC11_Debounce_Cnt = 0; // 编码器防抖计数

// Timer1
unsigned int Debounce_count = 0;
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
		flag_1ms = 1;
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

	TIM1ARRH = 0x7C; // ???????8
	TIM1ARRL = 0xFF; // ???????8λ  1ms ??????ж?

	INTCON = 0B11000000; // ??????ж???????ж?
}

/*-------------------------------------------------
 * 函数名：EC11_Read_State
 * 功能： 读取EC11编码器当前状态（A/B相组合）
 * 返回： 0x00(A=0,B=0) 0x01(A=0,B=1) 0x02(A=1,B=0) 0x03(A=1,B=1)
 --------------------------------------------------*/
uchar EC11_Read_State(void)
{
	uchar state = 0;
	if (EC11_B)
		state |= 0x02; // EC11_B=A相
	if (EC11_A)
		state |= 0x01; // EC11_A=B相
	return state;
}

/*-------------------------------------------------
 * 函数名：EC11_Process
 * 功能： 处理EC11编码器旋转逻辑（防抖+加减判断）
 --------------------------------------------------*/
void EC11_Process(void)
{
	uchar state_new = EC11_Read_State();

	// 步骤1：防抖——连续5ms状态不变才认为是稳定态
	if (state_new == EC11_State_Old)
	{
		if (EC11_Debounce_Cnt < 5) // 防抖时间5ms
		{
			EC11_Debounce_Cnt++;
		}
	}
	else
	{
		EC11_Debounce_Cnt = 0;
		EC11_State_Old = state_new;
		return;
	}

	// 步骤2：稳定态校验通过，判断旋转方向
	if (EC11_Debounce_Cnt == 5)
	{
		// EC11正转（A相超前B相）：0x03→0x02→0x00→0x01→0x03
		// EC11反转（B相超前A相）：0x03→0x01→0x00→0x02→0x03
		static uchar state_last = 0x03; // 初始状态
		if (state_new != state_last)
		{
			// 正转判断（顺时针，数值+1）
			if ((state_last == 0x03 && state_new == 0x02) ||
				(state_last == 0x02 && state_new == 0x00) ||
				(state_last == 0x00 && state_new == 0x01) ||
				(state_last == 0x01 && state_new == 0x03))
			{
				Number_Sum = (Number_Sum >= 99) ? 99 : (Number_Sum + 1);
			}
			// 反转判断（逆时针，数值-1）
			else if ((state_last == 0x03 && state_new == 0x01) ||
					 (state_last == 0x01 && state_new == 0x00) ||
					 (state_last == 0x00 && state_new == 0x02) ||
					 (state_last == 0x02 && state_new == 0x03))
			{
				Number_Sum = (Number_Sum <= 0) ? 0 : (Number_Sum - 1);
			}
			state_last = state_new; // 更新上一次状态
		}
		EC11_Debounce_Cnt++; // 避免重复触发
	}
}

void main(void)
{
	POWER_INITIAL(); // �0�3�0�1�1�7�1�7�0�3�1�7�1�7
	TIM1_INITIAL();
	TM1650_Init();
	TM1650_Set(led_place[0], seg_code[9]); // 能显示，测试
	EC11_State_Old = EC11_Read_State();
	while (1)
	{
		// 旋钮控制数据

		if (flag_1ms == 1) // 一秒进来执行一次
		{
			EC11_Process();
			flag_1ms = 0;
		}

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
}
//===========================================================
