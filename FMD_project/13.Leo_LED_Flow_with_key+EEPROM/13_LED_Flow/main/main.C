//Project: 13_LED_Flow.prj
// Device: FT61F0AX
// Memory: Flash 10KX14b, EEPROM 128X8b, SRAM 1KX8b
// Author: 
//Company: 
//Version:
//   Date: 
//===========================================================
//===========================================================
#include	"SYSCFG.h"
//===========================================================
//Variable definition
volatile char W_TMP  @ 0x70 ;
volatile char BSR_TMP  @ 0x71 ;
void user_isr();//�û��жϳ��򣬲���ɾ��
//===========================================================

//===========================================================
//Function name��interrupt ISR
//parameters����
//returned value����
//===========================================================
void interrupt ISR(void)
{
#asm;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
#endasm;
	user_isr(); 
}
void user_isr() 
{
}
void POWER_INITIAL(void)//PA全部输出，PB0和PB1配置为输入（已配置）
{
    OSCCON = 0B01100001; // 16MHz 2:1
    INTCON = 0;          

    // ********** GPIO **********
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

    //********************** GPIO **********************
    TRISA |= 0B00000000;
	TRISB |= 0B00000011;
    WPUA |= 0B00000000;
	WPUB |= 0B00000011; 

    // ********** 还不会，系统默认这样配的部分 **********
    PSRC0 = 0B11111111; 
    PSRC1 = 0B11111111;
    PSRC2 = 0B00001111;
    PSINK0 = 0B11111111; 
    PSINK1 = 0B11111111;
    PSINK2 = 0B00000011;
    ANSELA = 0B00000000; // 数字IO
}
//===========================================================
//Function name��main
//parameters����
//returned value����
//===========================================================
main()
{
	
	
	while(1)
	{
	
	}
}
//===========================================================
