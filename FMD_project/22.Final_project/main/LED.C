#include "SYSCFG.h"
#include "FT61F0AX.h"

#define u8 unsigned char
#define u16 unsigned int
#define LED_Left PA0
#define LED_Mid PA1
#define LED_Right PA3
// 打开LED
void LED_turn_on(void)
{

	LED_Right = 0;
	NOP();
	LED_Mid = 0;
	NOP();
	LED_Left = 0;
	NOP();
}

// 关闭LED
void LED_turn_off(void)
{
	LED_Right = 1;
	NOP();
	LED_Mid = 1;
	NOP();
	LED_Left = 1;
	NOP();
}

void LED_All_Shinning(void)
{
	LED_Left = ~LED_Left;
	LED_Mid = ~LED_Mid;
	LED_Right = ~LED_Right;
}

void LED_mid_right_on(void)
{
	LED_Right = 0;
	NOP();
	LED_Mid = 0;
	NOP();
}

void LED_Left_on(void)
{
	LED_Left = 0;
	NOP();
}
void LED_Left_off(void)
{
	LED_Left = 1;
	NOP();
}
//红灯闪烁
void LED_Left_shinning(void)
{
	LED_Left = ~LED_Left;
}