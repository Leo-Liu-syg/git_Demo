
#include "SYSCFG.h"
#include "FT64F0AX.h"
// #include "Key_NonBlock.h"
#include "URAT_INITIAL.h"
#include "delay.h"

volatile char W_TMP @0x70;
volatile char BSR_TMP @0x71;
#define EEPROM_FLAG_ADDR 0x00
#define EEPROM_FLAG_DATAH 0x01
#define EEPROM_FLAG_DATAL 0x02
#define PWM_ARR 1000
#define PWM_ARR_ONE 10

unsigned char flag_1ms = 0;

void POWER_INITIAL(void);
void TIM2_INITIAL(void);
void TIM1_INITIAL(void);
unsigned char EEPROMread(unsigned char EEAddr);
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);
void Key_Scan_NonBlock(void);

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

    // timer1�ж�
    if (T1UIF)
    {
        T1UIF = 1;
        flag_1ms = 1;
    }
}

void POWER_INITIAL(void)
{
    OSCCON = 0B01100001; // ???16MHz???????1:1
    INTCON = 0;          // ????????��?

    // ********** ???GPIO????? **********
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

    // ********** ??????PA0+PA1+PA7rtx ????????????????? **********
    TRISA |= 0B10000011;
    WPUA |= 0B10000011; // ?????????????????????????

    // ********** ??��??????? **********
    PSRC0 = 0B11111111; // ????????
    PSRC1 = 0B11111111;
    PSRC2 = 0B00001111;
    PSINK0 = 0B11111111; // ????????
    PSINK1 = 0B11111111;
    PSINK2 = 0B00000011;
    ANSELA = 0B00000000; // ???????IO
}

void TIM2_INITIAL(void)
{
    PCKEN |= 0B00000100; // ???TIMER2??????
    CKOCON = 0B00100000; // Timer2?????????????��4ns???
    TCKSRC = 0B00110000; // Timer2?????HIRC??2???

    TIM2CR1 = 0B10000101; // ???????????????????

    TIM2IER = 0B00000000; // ????????��?

    TIM2SR1 = 0B00000000;
    TIM2SR2 = 0B00000000;

    TIM2EGR = 0B00000000;

    TIM2CCMR1 = 0B01101000; // ?????CH1??????????PWM??1
    TIM2CCMR2 = 0B01101000; // ?????CH2??????????PWM??1
    TIM2CCMR3 = 0B00000000;

    TIM2CCER1 = 0B00110011; // ???1??2?????????????��
    TIM2CCER2 = 0B00000000;
    //*********AI ???????
    // TIM2CCMR1 = 0B01101010; // 01101010??T2OC1M=110??PWM1????T2OC1PE=1???????
    // TIM2CCMR2 = 0B01101010; // ???????CH2

    TIM2CNTRH = 0B00000000;
    TIM2CNTRL = 0B00000000;

    TIM2ARRH = (PWM_ARR >> 8); // ???????8��03H
    TIM2ARRL = PWM_ARR & 0xFF; // ???????8��e8H

    TIM2CCR1H = 0; // ?????1?????????8��01H
    TIM2CCR1L = 0; // ?????1?????????8��F4H

    TIM2CCR2H = 0x01; // ?????2?????????8��01H
    TIM2CCR2L = 0xf4; // ?????2?????????8��F4H

    TIM2CCR3H = 0B00000000;
    TIM2CCR3L = 0B00000000;
}
void TIM1_INITIAL(void)
{
    PCKEN |= 0B00000010;  // ???TIMER1??????
    CKOCON = 0B00100000;  // Timer1?????????????��4ns???
    TCKSRC |= 0B00000011; // Timer1?????HIRC??2???

    TIM1CR1 = 0B10000101; // ???????????????????

    TIM1IER = 0B00000001; // ?????????��?

    TIM1ARRH = 0x7C; // ???????8
    TIM1ARRL = 0xFF; // ???????8��  1ms ??????��?

    INTCON = 0B11000000; // ??????��???????��?
}

unsigned int ComValue1 = 0;
unsigned int ComValue1_buffer = 0;
unsigned char breath_Flag1 = 0; // 0:���� 1:����ռ�ձ� 2:����ռ�ձ�
unsigned char breath_count = 0;
unsigned char breath_mode = 0;
void berath_control()
{
    if (breath_Flag1 == 0)
    {
        breath_count = 0;
        if(ComValue1_buffer != ComValue1)
        {
            EEPROMwrite(EEPROM_FLAG_ADDR, 0x55);
            EEPROMwrite(EEPROM_FLAG_DATAL, ComValue1 / 256);
            EEPROMwrite(EEPROM_FLAG_DATAH, ComValue1 % 256);
            ComValue1_buffer = ComValue1;
        }   
    }
    // ÿ200ms����һ��ռ�ձ�
    else if (breath_Flag1 == 1)
    {
        breath_count++;
        if (breath_count >= 200)
        {
            breath_count = 0;
            if (ComValue1 < PWM_ARR)
            {
                ComValue1 += PWM_ARR_ONE;
                if (ComValue1 > PWM_ARR)
                {
                    ComValue1 = PWM_ARR;
                }
            }
            TIM2CCR1L = ComValue1 % 256;
            TIM2CCR1H = ComValue1 / 256;
        }
    }
    // ÿ200ms����һ��ռ�ձ�
    else if (breath_Flag1 == 2)
    {
        breath_count++;
        if (breath_count >= 200)
        {
            breath_count = 0;
            if (ComValue1 > 10)
            {
                ComValue1 -= 10;
            }
            else
            {
                ComValue1 = 0;
            }
            TIM2CCR1L = ComValue1 % 256;
            TIM2CCR1H = ComValue1 / 256;
        }
    }
}

unsigned char key_state_buffer = 0;
unsigned char key_state = 0;
unsigned char key_count = 0;
void Key_Scan_NonBlock(void)
{
    /*�������*/
    // ����������û����
    if (PA0 && PA1)
    {
        key_state_buffer = 0;
    }
    // ֻ����PA0
    else if (!PA0 && PA1)
    {
        key_state_buffer = 1;
    }
    // ֻ����PA1
    else if (PA0 && !PA1)
    {
        key_state_buffer = 2;
    }
    else if ((!PA0 && !PA1))
    {
        key_state_buffer = 3;
    }

    if (key_state != key_state_buffer)
    {
        key_count++;
        if (key_count >= 20)
        {
            key_count = 0;
            key_state = key_state_buffer;
            /*����״̬�л�����*/
            if (key_state == 0)
            {
                // PA4 = 1;
                // PA5 = 1;
                breath_Flag1 = 0;
            }
            else if (key_state == 1)
            {
                // PA4 = 0;
                // PA5 = 1;
                breath_Flag1 = 1;
            }
            else if (key_state == 2)
            {
                // PA4 = 1;
                // PA5 = 0;
                breath_Flag1 = 2;
            }
            else if (key_state == 3)
            {
                // PA4 = 0;
                // PA5 = 0;
                breath_Flag1 = 0;
            }
        }
    }
}
/*-------------------------------------------------
* ????????EEPROMread
* ?????  ??EEPROM????
* ????  EEAddr???????????
* ?????  ReEEPROMread????????????????
--------------------------------------------------*/
unsigned char EEPROMread(unsigned char EEAddr)
{
    unsigned char ReEEPROMread;
    while (GIE) // ???GIE?0
    {
        GIE = 0; // ????????????��?
        NOP();
        NOP();
    }
    EEADRL = EEAddr;

    CFGS = 0;
    EEPGD = 0;
    RD = 1;
    NOP();
    NOP();
    NOP();
    NOP();
    ReEEPROMread = EEDATL;
    GIE = 1;

    return ReEEPROMread;
}
/*-------------------------------------------------
 * ????????Unlock_Flash
 * ?????  ????FLASH/EEDATA???????????FLASH/EEDATA????????????
 *		   ?????????????????????????????
 * ????  ??
 * ?????  ??
 --------------------------------------------------*/
void Unlock_Flash()
{
#asm
    MOVLW 0x03;
    MOVWF _BSREG;
    MOVLW 0x55;
    MOVWF _EECON2;
    MOVLW 0xAA;
    MOVWF _EECON2;
    BSF _EECON1, 1;
    NOP;
    NOP;
#endasm
    //	#asm
    //		MOVLW 0x03 ;
    //        MOVWF _BSREG
    //		MOVLW 0x55 MOVWF _EECON2 &
    //		0x7F MOVLW 0xAA
    //        MOVWF _EECON2 & 0x7F
    //        BSF _EECON1 & 0x7F, 1 // WR=1;
    //		NOP
    //        NOP
    //	#endasm
}

void EEPROMwrite(unsigned char EEAddr, unsigned char Data)
{
    while (GIE) // ???GIE?0
    {
        GIE = 0; // ��??????????��?
        NOP();
        NOP();
    }
    EEADRL = EEAddr; // EEPROM????
    EEDATL = Data;   // EEPROM??????

    CFGS = 0;
    EEPGD = 0;
    WREN = 1; // ��???
    EEIF = 0;

    Unlock_Flash(); // Flash ????????????
    NOP();
    NOP();
    NOP();
    NOP();
    while (WR)
        ; // ???EEPROM��?????
    WREN = 0;
    GIE = 1;
}
/*-------------------------------------------------
 * ????????TIM2_INITIAL
 * ?????  ?????TIM2
 * ????  ??
 * ?????  ??
 */

void main(void)
{
    POWER_INITIAL();
    TIM2_INITIAL();
    TIM1_INITIAL();

    // ��д��EEPROM
    if(EEPROMread(EEPROM_FLAG_ADDR) == 0x55)
    {
        TIM2CCR1L = EEPROMread(EEPROM_FLAG_DATAL);
        TIM2CCR1H = EEPROMread(EEPROM_FLAG_DATAH);
    }

    while (1)
    {
        if (flag_1ms)
        {
            flag_1ms = 0;
            Key_Scan_NonBlock(); // ��������
            berath_control();
        }
    }
}
