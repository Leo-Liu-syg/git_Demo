#include "SYSCFG.h"
#include "FT61F0AX.h"

#define u8 unsigned char
#define u16 unsigned int
#define LED_Left PA1
#define LED_Mid PA3
#define LED_Right PA4
//打开LED
void LED_Buzz_turn_on(void)
{
	
	LED_Right = 0;
	LED_Mid = 0;
	LED_Left = 0;
}

//关闭LED
void LED_Buzz_turn_off(void)
{
	LED_Right = 1;
	NOP();
	LED_Mid = 1;
	NOP();
	LED_Left = 1;
	NOP();
}