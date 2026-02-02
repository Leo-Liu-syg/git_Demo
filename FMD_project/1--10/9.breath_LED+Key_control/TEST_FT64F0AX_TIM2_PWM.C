// Project: TEST_64F0Ax_TIM2_PWM.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description: 
//1.Á½¸ö°´¼ü£¬Ò»¸ö°´¼ü°´ÏÂÀ´µÄÊ±ºò£¬Ìá¸ßÁÁ¶È£
//          Ò»¸ö°´¼ü°´ÏÂÀ´½µµÍÁÁ¶È¡

//2.°´¼üËÉ¿ªÖ®ºóÒ»ÃëÖÓ£¬±£´æµ±Ç°ÁÁ¶Èµ½ eeprom£¬ÏÂ´ÎÉÏµçµÄÊ±ºò´Ó eeprom ¶ÁÈ¡¶ÔÓ¦ÁÁ¶È
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-5        ÐÞ¸ÄÏµÍ³Ê±ÖÓÎª8MHz£¬Ê¹ÄÜLVR
//                FT64F0A5  TSSOP20
//             -------------------
// TIM2_CH1----|1(PA5)   	(PA4)20|-----TIM2_CH2
// NC----------|2(PA6)   	(PA3)19|-----------NC
// NC----------|3(PA7)   	(PA2)18|-----------NC
// NC----------|4(PC0)   	(PA1)17|-----------NC
// NC----------|5(PC1)		(PA0)16|-----------NC
// NC----------|6(PB7)		(PB0)15|-----------NC
// GND---------|7(GND)		(PB1)14|-----------NC
// NC----------|8(PB6)		(PB2)13|-----------NC
// VDD---------|9(VDD)		(PB3)12|-----------NC
// NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
//
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "delay.h"

// *********************** Ô­ÓÐºê¶¨Òå ****************************
#define DemoPortOut PB3
#define T2_CH1_PIN  PA5  // TIM2_CH1¶ÔÓ¦PA5£¨LEDÒý½Å£©
// *********************** ÐÂÔö°´¼üºê¶¨Òå ****************************
#define KEY_PIN     PA0  // °´¼üÒý½Å£¨PA0£©
#define KEY_PRESSED 0    // °´¼ü°´ÏÂµçÆ½£¨ÉÏÀ­ÊäÈë£¬°´ÏÂÎªµÍ£©
#define KEY_RELEASED 1   // °´¼üÊÍ·ÅµçÆ½
#define DEBOUNCE_MS 20   // °´¼üÏû¶¶Ê±¼ä£¨20ms£©

// *********************** Ô­ÓÐ±äÁ¿¶¨Òå ****************************
volatile char W_TMP @0x70;	 // ÏµÍ³Õ¼ÓÃ£¬²»¿ÉÉ¾³ý
volatile char BSR_TMP @0x71; // ÏµÍ³Õ¼ÓÃ£¬²»¿ÉÉ¾³ý
unsigned char TempH1;
unsigned char TempL1;
unsigned int ComValue1;       // TIM2_CH1±È½ÏÖµ£¨¿ØÖÆÕ¼¿Õ±È£©
unsigned int T1;              // ºôÎüÖÜÆÚ²ÎÊý
unsigned char TempH2;
unsigned char TempL2;
unsigned int ComValue2;
unsigned int T2;
unsigned char Compare_Flag;
volatile unsigned char TIM1_Count; // TIM1ÖÐ¶Ï¼ÆÊý£¨1ms/´Î£¬volatileÈ·±£ÊµÊ±¸üÐÂ£©
unsigned char breath_Flag1 = 0;    // ºôÎü·½Ïò±êÖ¾£¨0=±ä°µ£¬1=±äÁÁ£©

// *********************** ÐÂÔöÄ£Ê½Óë°´¼ü±äÁ¿ ****************************
unsigned char breath_mode = 1;     // ºôÎüÄ£Ê½£º1=ºôÎü£¬0=ÔÝÍ£
// °´¼üÏû¶¶¾²Ì¬±äÁ¿£¨·Ç×èÈûÓÃ£©
static unsigned char key_last_level = KEY_RELEASED;  // ÉÏ´Î°´¼üµçÆ½
static unsigned int key_debounce_tick = 0;           // °´¼üÏû¶¶Ê±¼ä´Á£¨»ùÓÚTIM1_Count£©

// º¯ÊýÉùÃ÷
void user_isr(); 
void POWER_INITIAL(void);
void TIM2_INITIAL(void);
void TIM1_INITIAL(void);
unsigned char Key_Scan_NonBlock(void);


//===========================================================
// Funtion name£ºinterrupt ISR
// parameters£ºÎÞ
// returned value£ºÎÞ
//===========================================================
void interrupt ISR(void)
{
#asm;			// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	NOP;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
#endasm;		// ÏµÍ³ÉèÖÃ²»¿ÉÒÔÉ¾³ýºÍÐÞ¸Ä
	user_isr(); // µ÷ÓÃÓÃ»§ÖÐ¶Ïº¯Êý
}
void user_isr() // µ÷ÓÃÓÃ»§ÖÐ¶Ïº¯Êý
{
	T1UIF = 1;		// Ð´1ÇåÁã±êÖ¾Î»
	PORTB = ~PORTB; // ·­×ªµçÆ½
	TIM1_Count++;
}
/*-------------------------------------------------
 * º¯ÊýÃû£ºPOWER_INITIAL
 * ¹¦ÄÜ£º  ÉÏµçÏµÍ³³õÊ¼»¯
 * ÊäÈë£º  ÎÞ
 * Êä³ö£º  ÎÞ
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // ÄÚ²¿16MHzÊ±ÖÓ£¬·ÖÆµ1:1
	INTCON = 0;			 // ½ûÖ¹ËùÓÐÖÐ¶Ï

	// ********** Ô­ÓÐGPIO³õÊ¼»¯ **********
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

	// ********** ÐÂÔö£ºPA0°´¼üÅäÖÃ£¨ÉÏÀ­ÊäÈë£© **********
	TRISA |= 0B00000001; 
	WPUA |= 0B00000001;   // ¿ªÆôPA0ÈõÉÏÀ­£¨·ÀÖ¹¸¡¿ÕÎó´¥·¢£©

	// ********** Ô­ÓÐµçÁ÷ÅäÖÃ **********
	PSRC0 = 0B11111111; // Ô´µçÁ÷×î´ó
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;
	PSINK0 = 0B11111111; // ¹àµçÁ÷×î´ó
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;
	ANSELA = 0B00000000; // PA0ÉèÎªÊý×ÖIO
}
/**
 * ÐÞ¸´ºóµÄ·Ç×èÈû°´¼üÉ¨Ãè£¨PA0£©
 * ·µ»ØÖµ£º1=°´¼ü°´ÏÂ£¨Ïû¶¶ºó£¬½ö´¥·¢Ò»´Î£©£¬0=ÎÞ°´¼ü/Î´ÎÈ¶¨
 */


void TIM2_INITIAL(void)
{
	PCKEN |= 0B00000100; // Ê¹ÄÜTIMER2Ä£¿éÊ±ÖÓ
	CKOCON = 0B00100000; // Timer2±¶ÆµÊ±ÖÓÕ¼¿Õ±Èµ÷½ÚÎ»4nsÑÓ³Ù
	TCKSRC = 0B00110000; // Timer2Ê±ÖÓÔ´ÎªHIRCµÄ2±¶Æµ

	TIM2CR1 = 0B10000101; // ÔÊÐí×Ô¶¯×°ÔØ£¬Ê¹ÄÜ¼ÆÊýÆ÷

	TIM2IER = 0B00000000; // ½ûÖ¹ËùÓÐÖÐ¶Ï

	TIM2SR1 = 0B00000000;
	TIM2SR2 = 0B00000000;

	TIM2EGR = 0B00000000;

	TIM2CCMR1 = 0B01101000; // ½«Í¨µÀCH1ÅäÖÃÎªÊä³ö£¬PWMÄ£Ê½1
	TIM2CCMR2 = 0B01101000; // ½«Í¨µÀCH2ÅäÖÃÎªÊä³ö£¬PWMÄ£Ê½1
	TIM2CCMR3 = 0B00000000;

	TIM2CCER1 = 0B00110011; // ±È½Ï1ºÍ2Êä³öÊ¹ÄÜ£¬µÍµçÆ½ÓÐÐ§
	TIM2CCER2 = 0B00000000;

	TIM2CNTRH = 0B00000000;
	TIM2CNTRL = 0B00000000;

	TIM2ARRH = 0x03; // ×Ô¶¯×°ÔØ¸ß8Î»03H
	TIM2ARRL = 0xe8; // ×Ô¶¯×°ÔØµÍ8Î»e8H

	TIM2CCR1H = 0x01; // ×°Èë±È½Ï1µÄÔ¤×°ÔØÖµ¸ß8Î»01H
	TIM2CCR1L = 0xf4; // ×°Èë±È½Ï1µÄÔ¤×°ÔØÖµµÍ8Î»F4H

	TIM2CCR2H = 0x01; // ×°Èë±È½Ï2µÄÔ¤×°ÔØÖµ¸ß8Î»01H
	TIM2CCR2L = 0xf4; // ×°Èë±È½Ï2µÄÔ¤×°ÔØÖµµÍ8Î»F4H

	TIM2CCR3H = 0B00000000;
	TIM2CCR3L = 0B00000000;
}
/*-------------------------------------------------
* º¯ÊýÃû£ºTIM1_INITIAL
* ¹¦ÄÜ£º  ³õÊ¼»¯TIM1
* ÊäÈë£º  ÎÞ
* Êä³ö£º  ÎÞ
--------------------------------------------------*/
void TIM1_INITIAL(void)
{
	PCKEN |= 0B00000010;  // Ê¹ÄÜTIMER1Ä£¿éÊ±ÖÓ
	CKOCON = 0B00100000;  // Timer1±¶ÆµÊ±ÖÓÕ¼¿Õ±Èµ÷½ÚÎ»4nsÑÓ³Ù
	TCKSRC |= 0B00000011; // Timer1Ê±ÖÓÔ´ÎªHIRCµÄ2±¶Æµ

	TIM1CR1 = 0B10000101; // ÔÊÐí×Ô¶¯×°ÔØ£¬Ê¹ÄÜ¼ÆÊýÆ÷

	TIM1IER = 0B00000001; // ÔÊÐí¸üÐÂÖÐ¶Ï

	TIM1ARRH = 0x7C; // ×Ô¶¯×°ÔØ¸ß8
	TIM1ARRL = 0xFF; // ×Ô¶¯×°ÔØµÍ8Î»  1ms ½øÒ»´ÎÖÐ¶Ï

	INTCON = 0B11000000; // Ê¹ÄÜ×ÜÖÐ¶ÏºÍÍâÉèÖÐ¶Ï
}
/*-------------------------------------------------
 * º¯ÊýÃû£ºmain
 * ¹¦ÄÜ£º  Ö÷º¯Êý
 * ÊäÈë£º  ÎÞ
 * Êä³ö£º  ÎÞ
 --------------------------------------------------*/
unsigned char Key_Scan_NonBlock(void)
{
    static unsigned char key_state = 0; // 0=ÊÍ·Å£¬1=Ïû¶¶ÖÐ£¬2=°´ÏÂ
    unsigned char key_current_level = (PORTA & 0B00000001); // ¶ÁÈ¡PA0µçÆ½

    switch(key_state)
    {
        case 0: // ÊÍ·Å×´Ì¬£¬¼ì²â°´ÏÂ
            if(key_current_level == KEY_PRESSED)
            {
                key_debounce_tick = TIM1_Count; // ¼ÇÂ¼Ïû¶¶¿ªÊ¼Ê±¼ä
                key_state = 1; // ½øÈëÏû¶¶ÖÐ×´Ì¬
            }
            break;
        
        case 1: // Ïû¶¶ÖÐ£¬ÅÐ¶ÏÊÇ·ñÎÈ¶¨°´ÏÂ20ms
            // ´¦ÀíTIM1_CountÒç³ö£¨8Î»±äÁ¿£¬Òç³öºó²îÖµÎª¸º£¬Ðè×ª»»ÎªÎÞ·ûºÅ£©
            if((unsigned char)(TIM1_Count - key_debounce_tick) >= DEBOUNCE_MS)
            {
                if(key_current_level == KEY_PRESSED)
                {
                    key_state = 2; // È·ÈÏ°´ÏÂ
                    return 1; // ·µ»Ø°´ÏÂÊÂ¼þ
                }
                else
                {
                    key_state = 0; // µçÆ½»Øµ¯£¬»Øµ½ÊÍ·Å×´Ì¬
                }
            }
            break;
        
        case 2: // °´ÏÂ×´Ì¬£¬¼ì²âÊÍ·Å
            if(key_current_level == KEY_RELEASED)
            {
                key_debounce_tick = TIM1_Count;
                key_state = 3; // ½øÈëÊÍ·ÅÏû¶¶
            }
            break;
        
        case 3: // ÊÍ·ÅÏû¶¶£¬ÎÈ¶¨ºó¸´Î»
            if((unsigned char)(TIM1_Count - key_debounce_tick) >= DEBOUNCE_MS)
            {
                key_state = 0; // »Øµ½ÊÍ·Å×´Ì¬
            }
            break;
    }
    return 0;
}
/*-------------------------------------------------
* º¯ÊýÃû£ºTIM2_INITIAL
* ¹¦ÄÜ£º  ³õÊ¼»¯TIM2
* ÊäÈë£º  ÎÞ
* Êä³ö£º  ÎÞ
*/
void main(void)
{
    POWER_INITIAL(); 
    TIM2_INITIAL();  
    TIM1_INITIAL();  

    // ³õÊ¼»¯²ÎÊý
    TempH1 = TIM2CCR1H;
    TempL1 = TIM2CCR1L;
    ComValue1 = TempH1 * 256 + TempL1;  
    T1 = 2 * (TempH1 * 256 + TempL1) - 1;
    Compare_Flag = 0;
    static unsigned int last_breath_tick = 0; // ºôÎü¸üÐÂÊ±¼ä´Á

    while (1)
    {
        // 1. °´¼üÉ¨Ãè£ºÇÐ»»ºôÎü/ÔÝÍ£Ä£Ê½£¨¸ßÆµµ÷ÓÃ£¬ÎÞ×èÈû£©
        if (Key_Scan_NonBlock() == 1)
        {
            breath_mode = !breath_mode; 
            // Ìí¼Óµ÷ÊÔ´òÓ¡£¨ÐèÅäÖÃ´®¿Ú£©£¬È·ÈÏÄ£Ê½ÇÐ»»
            // printf("breath_mode: %d\n", breath_mode);
        }

        // 2. ºôÎüÄ£Ê½£º»ùÓÚÊ±¼ä´Á¸üÐÂ£¬ÎÞ×èÈû
        if (breath_mode == 1)
        {
            // Ã¿50ms¸üÐÂÒ»´ÎÕ¼¿Õ±È£¨ÓëÔ­Âß¼­Ò»ÖÂ£©
            if((unsigned char)(TIM1_Count - last_breath_tick) > 49)
            {
                last_breath_tick = TIM1_Count; // ¸üÐÂÊ±¼ä´Á

                if(breath_Flag1 == 1) // ±äÁÁ
                {
                    ComValue1 += 20;
                    if (ComValue1 >= T1)
                    {
                        breath_Flag1 = 0;
                    }
                }
                else // ±ä°µ
                {
                    ComValue1 -= 20;
                    if (ComValue1 <= 0)
                    {
                        breath_Flag1 = 1;
                    }
                }
                // ¸üÐÂPWM±È½ÏÖµ
                TIM2CCR1H = ComValue1 / 256;
                TIM2CCR1L = ComValue1 % 256;
            }
        }
        // ÔÝÍ£Ä£Ê½£ºÎÞ²Ù×÷£¬Î¬³Öµ±Ç°Õ¼¿Õ±È
        NOP();
    }
}