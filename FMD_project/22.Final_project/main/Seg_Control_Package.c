#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "TDelay.h"
#include "TM1650_IIC_1.h"
#include "TM1650_IIC_2.h"

#define S 0
#define H 1

// 数码管变量定义
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

// 数码管1
volatile float Number_Sum_1 = 0.0f;
float Number_Sum_1_Old = 0.0f;
float Number_Sum_1_Abs = 0.0f; // 用于处理负数
int Number_Ge_1 = 0;
int Number_Shi_1 = 0;
int Number_Bai_1 = 0;
int Number_Qian_1 = 0;
char Number_Dec_1 = 0;

// 数码管2
volatile float Number_Sum_2 = 0.0f;
float Number_Sum_2_Old = 0.0f;
float Number_Sum_2_Abs = 0.0f; // 用于处理负数
int Number_Ge_2 = 0;
int Number_Shi_2 = 0;
int Number_Bai_2 = 0;
int Number_Qian_2 = 0;
char Number_Dec_2 = 0;


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
			TM1650_1_Set(led_place[3], 0b00111001); // H:0b01110110
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

void Seg1_Init_Ready(void)
{
	TM1650_1_Set(led_place[0], 0xFF);
	TM1650_1_Set(led_place[1], 0xFF);
	TM1650_1_Set(led_place[2], 0xFF);
	TM1650_1_Set(led_place[3], 0xFF); // H:0b01110110
}

void Seg2_Init_Ready(void)
{
	TM1650_2_Set(led_place[0], 0xFF);
	TM1650_2_Set(led_place[1], 0xFF);
	TM1650_2_Set(led_place[2], 0xFF);
	TM1650_2_Set(led_place[3], 0xFF); // H:0b01110110
}

