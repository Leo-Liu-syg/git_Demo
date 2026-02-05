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
#include "TM1650_IIC_1.h"
#include "TM1650_IIC_2.h"
#include "EC11.h"
#include "IIC_SHT.h"
//===========================================================

//***********************宏定义****************************
#define uint unsigned int
#define ulong unsigned long
#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8�����ȣ�7����ʾ��������ʾʹ�ܣ��ֲ�����ˣ�
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL_seg1 PB3
#define SDA_seg1 PB4

#define SCL_seg2 PA0
#define SDA_seg2 PA1

#define SDA_SHT PA4
#define SCL_SHT PA5

#define EC11_A PA7
#define EC11_B PA6

// 数码管变量定义
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};
// 温度数码管1
float Number_Sum_1 = 0.0f;
float Number_Sum_1_Old = 0.0f;
float Number_Sum_1_Abs = 0.0f; // 用于处理负数
int Number_Ge_1 = 0;
int Number_Shi_1 = 0;
int Number_Bai_1 = 0;
int Number_Qian_1 = 0;
char Number_Dec_1 = 0;
// 湿度数码管2
float Number_Sum_2 = 0.0f;
float Number_Sum_2_Old = 0.0f;
float Number_Sum_2_Abs = 0.0f; // 用于处理负数
int Number_Ge_2 = 0;
int Number_Shi_2 = 0;
int Number_Bai_2 = 0;
int Number_Qian_2 = 0;
char Number_Dec_2 = 0;

// // LED_Breath
// unsigned char LED_Count = 0;
// unsigned char LED_PWM_Mid = 0;
// unsigned char Breath_Flag1 = 0;

// 按键扫描消抖

// unsigned char key_statue_buffer = 0;
// unsigned char key_statue = 0;
// unsigned char key_count = 0;
// unsigned char PWM_count = 0;

// 编码器相关变量
unsigned char EC11_State_Old = 0;	 // 编码器上一次状态（A/B相组合）
unsigned char EC11_Debounce_Cnt = 0; // 编码器防抖计数

// Timer1
unsigned int Debounce_count = 0;
unsigned int Timer1_count1 = 0;
unsigned int Timer1_count2 = 0;
unsigned int Timer1_count3 = 0;

// 三个不同时间的旗帜
unsigned char flag_1s = 0;
unsigned char Seg2_flag = 0;
unsigned char Seg1_flag = 0;

// 温湿度
//  1. 定义全局/局部缓冲区（6个元素，存传感器返回的6字节原始数据）
unsigned int sht_data_buf[6] = {0}; // 缓冲区初始化清零
// 2. 定义变量存最终实际温湿度（浮点型，保留2位小数足够）
float sht_temperature = 0.0f; // 温度(℃)
float sht_humidity = 0.0f;	  // 湿度(%RH)
unsigned int temp_raw = 0;
unsigned int humi_raw = 0;

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
		if (Timer1_count1 >= 20000) // 1s
		{
			flag_1s = 1;
			Timer1_count1 = 0;
		}
		if (Timer1_count2 >= 3000) // 没有用上
		{
			Seg2_flag = 1;
			Timer1_count2 = 0;
		}
		if (Timer1_count3 >= 3000) // 150ms
		{
			Seg1_flag = 1;
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
	POWER_INITIAL(); // �0�3�0�1�1�7�1�7�0�3�1�7�1�7
	TIM1_INITIAL();
	TM1650_1_Init();
	TM1650_2_Init();
	TM1650_1_Set(led_place[3], 0b00111001); // 能显示，测试
	TM1650_2_Set(led_place[3], 0b01110110);
	// EC11_State_Old = EC11_Read_State();
	while (1)
	{

		if (flag_1s == 1) // 1s进来执行一次
		{
			SHT_process();

			// 数据处理
			//  4. 拼接16位原始数据（高字节+低字节，SHT系列温湿度均为16位数据）
			temp_raw = (sht_data_buf[0] << 8) | sht_data_buf[1]; // 温度原始值（第0、1字节）
			humi_raw = (sht_data_buf[3] << 8) | sht_data_buf[4]; // 湿度原始值（第3、4字节）
			// （可选）CRC校验：对比sht_data_buf[2]（温度CRC）、sht_data_buf[5]（湿度CRC），提升数据可靠性

			// 5. 转换为实际温湿度（SHT3x/SHT2x通用公式，手册标准转换方法）
			sht_temperature = (float)(temp_raw * 175.0f / 65535.0f) - 45.0f; // 温度公式：-45~125℃
			sht_humidity = (float)(humi_raw * 100.0f / 65535.0f);			 // 湿度公式：0~100%RH
			// sht_temperature=-16.123;
			Number_Sum_1 = sht_temperature;
			Number_Sum_2 = (sht_humidity > 100) ? 100.0f : sht_humidity; // 裁剪超范围值
			flag_1s = 0;
		}
		if (Seg2_flag == 1) // 150ms
		{
			if (Number_Sum_2_Old != Number_Sum_2)
			{
				Seg2_Display();
				Number_Sum_2_Old = Number_Sum_2;
			}
			Seg2_flag = 0;
		}

		if (Seg1_flag == 1) // 150ms
		{
			if (Number_Sum_1_Old != Number_Sum_1)
			{
				Seg1_Display(); // 里面有Delay
				Number_Sum_1_Old = Number_Sum_1;
			}
			Seg1_flag = 0;
		}
	}
}
//===========================================================
