// Project: project.prj
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
//===========================================================
// Variable definition
unsigned int TIM1_Count = 0;
unsigned char Key_Mode = 0;
unsigned char Intermediate = 0;
volatile char W_TMP @0x70;   // ϵͳռ�ò�����ɾ�����޸�
volatile char BSR_TMP @0x71; // ϵͳռ�ò�����ɾ�����޸�
unsigned char Delay_20ms_Count = 0;
void user_isr(); // �û��жϳ��򣬲���ɾ��
//===========================================================

//===========================================================
// Function name��interrupt ISR
// parameters����
// returned value����
//===========================================================
void interrupt ISR(void)
{
#asm;           // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
#endasm;        // ϵͳ���ò�����ɾ�����޸�
    user_isr(); // �û��жϺ���
}

void user_isr() // �û��жϺ���
{
    Delay_20ms_Count++;
    if (T1UIE && T1UIF)
    {
        if (Key_Mode == 0)
        {
            if (TIM1_Count < 1000)
            {
                TIM1_Count++;
                PA5 = 0;
            } // LED ��
            else if (TIM1_Count >= 1000 && TIM1_Count < 2000)
            {
                TIM1_Count++;
                PA5 = 1;
            } // LED ��
            else if (TIM1_Count >= 2000)
            {
                TIM1_Count = 0;
                PA5 = 0;
            }
        } // ����
        if (Key_Mode == 1)
        {
            if (TIM1_Count < 2000)
            {
                TIM1_Count++;
                PA5 = 0;
            } // LED ��
            else if (TIM1_Count >= 2000 && TIM1_Count < 4000)
            {
                TIM1_Count++;
                PA5 = 1;
            } // LED ��
            else if (TIM1_Count >= 4000)
            {
                TIM1_Count = 0;
                PA5 = 0;
            }
        } // ����
        if (Key_Mode == 2)
        {
            if (TIM1_Count < 500)
            {
                TIM1_Count++;
                PA5 = 0;
            } // LED ��
            else if (TIM1_Count >= 500 && TIM1_Count < 1000)
            {
                TIM1_Count++;
                PA5 = 1;
            } // LED ��
            else if (TIM1_Count >= 1000)
            {
                TIM1_Count = 0;
                PA5 = 0;
            }
        } // ����
        if (Key_Mode == 3)
        {
            if (TIM1_Count < 100)
            {
                TIM1_Count++;
                PA5 = 0;
            } // LED ��
            else if (TIM1_Count >= 100 && TIM1_Count < 200)
            {
                TIM1_Count++;
                PA5 = 1;
            } // LED ��
            else if (TIM1_Count >= 200)
            {
                TIM1_Count = 0;
                PA5 = 0;
            }
        } // ����

        T1UIF = 1;
    }
}
//===========================================================
// Function name��main
// parameters����
// returned value����
//===========================================================
void POWER_INITIAL(void)
{
    OSCCON = 0B01100001; // ϵͳʱ��ѡ��Ϊ�ڲ�����8MHz, ��Ƶ��Ϊ1:1
    INTCON = 0;          // ��ֹ�����ж�

    PORTA = 0B00000000;
    PORTB = 0B00000000;
    PORTC = 0B00000000;

    WPUA = 0B00000000; // �������Ŀ��أ�0-�أ�1-��
    WPUB = 0B00000000;
    WPUC = 0B00000000;

    WPDA = 0B00000000; // �������Ŀ��أ�0-�أ�1-��
    WPDB = 0B00000000;
    WPDC = 0B00000000;

    TRISA = 0B00010000; // PA���������0-�����1-���� PA4-����,PA5���
    TRISB = 0B00000000; // PB���������0-�����1-����
    TRISC = 0B00000000;

    PSRC0 = 0B11111111; // Դ�����������
    PSRC1 = 0B11111111;
    PSRC2 = 0B00001111;

    PSINK0 = 0B11111111; // ������������
    PSINK1 = 0B11111111;
    PSINK2 = 0B00000011;

    ANSELA = 0B00000000; // ���ö�Ӧ��IOΪ����IO
}
void PA4_Init(void)
{
    // 1. ����PA4Ϊ����ģʽ
    TRISA |= (1 << 4); // PADIR��bit4���㣨0=���룩
    // 2. ����PA4���ڲ���������
    WPUA |= (1 << 4); // PAPUEN��bit4��1��ʹ��������
}
void PA5_Init(void)
{
    // 1. ����PA5Ϊ���ģʽ
    TRISA &= ~(1 << 5);
}
unsigned char PA4_Read_Key(void)
{
    // ��ȡPA4�ĵ�ƽ��0=�������£�1=����δ����
    if ((PORTA & (1 << 4)) == 0)
    {
        return 1; // ��������
    }
    else
    {
        return 0; // ����δ����
    }
}
void TIM1_INITIAL(void)
{
    PCKEN |= 0B00000010; // ʹ��TIMER1ģ��ʱ��
    CKOCON = 0B00100000; // Timer1��Ƶʱ��ռ�ձȵ���λ4ns�ӳ�
    TCKSRC = 0B00000011; // Timer1ʱ��ԴΪHIRC��2��Ƶ

    TIM1CR1 = 0B10000101; // �����Զ�װ�أ�ʹ�ܼ�����

    TIM1IER = 0B00000001; // ���������ж�

    TIM1ARRH = 0x3E; // �Զ�װ�ظ�8
    TIM1ARRL = 0x7F; // �Զ�װ�ص�8

    INTCON = 0B11000000; // ʹ�����жϺ������ж�
}

main()
{
    POWER_INITIAL();
    PA4_Init();
    PA5_Init();
    TIM1_INITIAL();
    while (1)
    {
        // ��һ������⵽�������£���ʼ��ƽ�źţ�
        if (PA4_Read_Key() == 1)
        {
            // �ڶ�����������ʱ����������ʼ�ۼ�20ms
            Delay_20ms_Count = 0;

            // ���������ȴ��ۼ�20ms������������ͬʱִ�������߼���
            while (Delay_20ms_Count < 20)
                ;

            // ���Ĳ�����ʱ���ٴ�ȷ�ϰ����Ƿ��԰��£���Ч�����жϣ�
            if (PA4_Read_Key() == 1)
            {
                // ���岽��ִ�а���ģʽ�л��߼���ԭ�д��룩
                if (Key_Mode < 3)
                {
                    Intermediate += 1;
                    Key_Mode = Intermediate;
                }
                else
                {
                    Intermediate = 0;
                    Key_Mode = 0;
                }

                // ���������ȴ������ͷţ������ظ�����
                while (PA4_Read_Key() == 1)
                    ;
            }
        }
    }
}
//===========================================================
