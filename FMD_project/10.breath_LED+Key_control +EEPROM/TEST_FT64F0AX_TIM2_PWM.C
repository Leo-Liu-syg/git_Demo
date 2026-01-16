// Project: TEST_64F0Ax_TIM2_PWM.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description:
// 1.两个按键，一个按键按下来的时候，提高亮度?
//          一个按键按下来降低亮度?

// 2.按键松开之后一秒钟，保存当前亮度到 eeprom，下次上电的时候从 eeprom 读取对应亮度
//
//  RELEASE HISTORY
//  VERSION DATE     DESCRIPTION
//  1.6        25-6-5        修改系统时钟为8MHz，使能LVR
//                 FT64F0A5  TSSOP20
//              -------------------
//  TIM2_CH1----|1(PA5)   	(PA4)20|-----TIM2_CH2
//  NC----------|2(PA6)   	(PA3)19|-----------NC
//  NC----------|3(PA7)   	(PA2)18|-----------NC
//  NC----------|4(PC0)   	(PA1)17|-----------NC
//  NC----------|5(PC1)		(PA0)16|-----------NC
//  NC----------|6(PB7)		(PB0)15|-----------NC
//  GND---------|7(GND)		(PB1)14|-----------NC
//  NC----------|8(PB6)		(PB2)13|-----------NC
//  VDD---------|9(VDD)		(PB3)12|-----------NC
//  NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
//
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
// #include "Key_NonBlock.h"
#include "URAT_INITIAL.h"
// #include "delay.h"

// *********************** 原有宏定义 ****************************
#define DemoPortOut PB3
#define T2_CH1_PIN PA5 // TIM2_CH1对应PA5（LED引脚）
// #define KEY_PIN     PA0  // 按键引脚（PA0）
#define KEY0_PRESSED 0B10000010 // 按键0按下电平（上拉输入，按下为低）
#define KEY1_PRESSED 0B10000001 // 按键0按下电平（上拉输入，按下为低）
#define KEY_RELEASED 0B10000011 // 按键释放电平
#define DEBOUNCE_MS 20          // 按键消抖时间（20ms）

// *********************** 原有呼吸灯相关定义 ****************************
volatile char W_TMP @0x70;   // 系统占用，不可删除
volatile char BSR_TMP @0x71; // 系统占用，不可删除
unsigned char TempH1;
unsigned char TempL1;
unsigned int ComValue1; // TIM2_CH1比较值（控制占空比）
unsigned int T1;        // 呼吸周期参数
unsigned char TempH2;
unsigned char TempL2;
unsigned int ComValue2;
unsigned int T2;
unsigned char Compare_Flag;
volatile unsigned int TIM1_Count; // TIM1中断计数（1ms/次，volatile确保实时更新）
unsigned char breath_Flag1 = 0;   // 呼吸方向标志（0=变暗，1=变亮）
unsigned char breath_mode = 0;
// 按键消抖静态变量（非阻塞用）
static unsigned char key_last_level = KEY_RELEASED; // 上次按键电平
static unsigned int key_debounce_tick = 0;          // 按键消抖时间戳（基于TIM1_Count）
// *********************** 移植串口相关定义 ****************************
volatile unsigned char receivedata = 0;
volatile unsigned char sendaddress = 0;
volatile unsigned char senddata = 0;
volatile unsigned char EEReadData = 0xAA;
unsigned char send_flag = 0;
unsigned char init_send_done = 0;

// 函数声明
void user_isr();
void POWER_INITIAL(void);
void TIM2_INITIAL(void);
void TIM1_INITIAL(void);
unsigned char Key_Scan_NonBlock(void);
unsigned char EEPROMread(unsigned char EEAddr);
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);
//===========================================================
// Funtion name：interrupt ISR
// parameters：无
// returned value：无
//===========================================================
void interrupt ISR(void)
{
#asm;           // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
    NOP;        // 系统设置不可以删除和修改
#endasm;        // 系统设置不可以删除和修改
    user_isr(); // 调用用户中断函数
}
void user_isr() // 调用用户中断函数
{
    T1UIF = 1; // 写1清零标志位
    // PORTB = ~PORTB; // 翻转电平
    TIM1_Count++;

    //----------------串口接收from电脑----------------------------------
    if (UR1RXNE && UR1RXNEF)
    {
        UR1RXNEF = 0;
        EEReadData = UR1DATAL;
        if (EEReadData == 0xAA)
        {
            EEReadData = EEPROMread(0x13); // 读原来的ROM

            EEPROMwrite(0x13, EEReadData + 1); // 写入ROM
        }
        else if (EEReadData == 0xBB)
        {
            EEReadData = EEPROMread(0x13);     // 自减
            EEPROMwrite(0x13, EEReadData - 1); // 写入ROM
                                               // ROM存入readdata
        }
        else if (EEReadData >= 0xff)
        {
            EEPROMwrite(0x13, 0x03);
        }
        else
        {
            EEPROMwrite(0x13, EEReadData);
        }
        send_flag = 1;
    }

    if (UR1TCEN && UR1TCF) // 串口发送回电脑
    {
        UR1TCF = 0;
        if (send_flag == 1)
        {
            senddata = EEPROMread(0x13);
            UR1DATAL = senddata;
            send_flag = 0;
        }
    }
}
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
    OSCCON = 0B01100001; // 内部16MHz时钟，分频1:1
    INTCON = 0;          // 禁止所有中断

    // ********** 原有GPIO初始化 **********
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

    // ********** 新增：PA0+PA1+PA7rtx 按键配置（上拉输入） **********
    TRISA |= 0B10000011;
    WPUA |= 0B10000011; // 开启弱上拉（防止浮空误触发）

    // ********** 原有电流配置 **********
    PSRC0 = 0B11111111; // 源电流最大
    PSRC1 = 0B11111111;
    PSRC2 = 0B00001111;
    PSINK0 = 0B11111111; // 灌电流最大
    PSINK1 = 0B11111111;
    PSINK2 = 0B00000011;
    ANSELA = 0B00000000; // 设为数字IO
}
/**
 * 修复后的非阻塞按键扫描（PA0）
 * 返回值：1=按键按下（消抖后，仅触发一次），0=无按键/未稳定
 */

void TIM2_INITIAL(void)
{
    PCKEN |= 0B00000100; // 使能TIMER2模块时钟
    CKOCON = 0B00100000; // Timer2倍频时钟占空比调节位4ns延迟
    TCKSRC = 0B00110000; // Timer2时钟源为HIRC的2倍频

    TIM2CR1 = 0B10000101; // 允许自动装载，使能计数器

    TIM2IER = 0B00000000; // 禁止所有中断

    TIM2SR1 = 0B00000000;
    TIM2SR2 = 0B00000000;

    TIM2EGR = 0B00000000;

    TIM2CCMR1 = 0B01101000; // 将通道CH1配置为输出，PWM模式1
    TIM2CCMR2 = 0B01101000; // 将通道CH2配置为输出，PWM模式1
    TIM2CCMR3 = 0B00000000;

    TIM2CCER1 = 0B00110011; // 比较1和2输出使能，低电平有效
    TIM2CCER2 = 0B00000000;
    //*********AI 改动如下
    // TIM2CCMR1 = 0B01101010; // 01101010：T2OC1M=110（PWM1），T2OC1PE=1（预装载）
    // TIM2CCMR2 = 0B01101010; // 同理配置CH2

    TIM2CNTRH = 0B00000000;
    TIM2CNTRL = 0B00000000;

    TIM2ARRH = 0x03; // 自动装载高8位03H
    TIM2ARRL = 0xe8; // 自动装载低8位e8H

    TIM2CCR1H = 0x01; // 装入比较1的预装载值高8位01H
    TIM2CCR1L = 0xf4; // 装入比较1的预装载值低8位F4H

    TIM2CCR2H = 0x01; // 装入比较2的预装载值高8位01H
    TIM2CCR2L = 0xf4; // 装入比较2的预装载值低8位F4H

    TIM2CCR3H = 0B00000000;
    TIM2CCR3L = 0B00000000;
}
/*-------------------------------------------------
* 函数名：TIM1_INITIAL
* 功能：  初始化TIM1
* 输入：  无
* 输出：  无
--------------------------------------------------*/
void TIM1_INITIAL(void)
{
    PCKEN |= 0B00000010;  // 使能TIMER1模块时钟
    CKOCON = 0B00100000;  // Timer1倍频时钟占空比调节位4ns延迟
    TCKSRC |= 0B00000011; // Timer1时钟源为HIRC的2倍频

    TIM1CR1 = 0B10000101; // 允许自动装载，使能计数器

    TIM1IER = 0B00000001; // 允许更新中断

    TIM1ARRH = 0x7C; // 自动装载高8
    TIM1ARRL = 0xFF; // 自动装载低8位  1ms 进一次中断

    INTCON = 0B11000000; // 使能总中断和外设中断
}
/*-------------------------------------------------
 * 函数名：main
 * 功能：  主函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
unsigned char Key_Scan_NonBlock(void)
{
    static unsigned char key_state = 0;                     // 0=释放，1=消抖中，2=按下
    unsigned char key_current_level = (PORTA & 0B10000011); // 读取PA0/PA1电平
    unsigned char Ret_Val = 0;
    static unsigned int key_debounce_tick = 0;
    switch (key_state)
    {
    case 0: // 释放状态，检测按下
        if (key_current_level == KEY0_PRESSED || key_current_level == KEY1_PRESSED)
        {
            key_debounce_tick = TIM1_Count; // 记录消抖开始时间
            key_state = 1;                  // 进入消抖中状态
        }
        break;

    case 1: // 消抖中，判断是否稳定按下20ms
        // 处理TIM1_Count溢出（8位变量，溢出后差值为负，需转换为无符号）
        if ((unsigned char)(TIM1_Count - key_debounce_tick) >= DEBOUNCE_MS)
        {
            if (key_current_level == KEY0_PRESSED)
            {
                key_state = 2;          // 确认按下
                Ret_Val = KEY0_PRESSED; // 返回按下事件
            }
            else if (key_current_level == KEY1_PRESSED)
            {
                key_state = 2;          // 确认按下
                Ret_Val = KEY1_PRESSED; // 返回按下事件
            }
            else
            {
                key_state = 0; // 电平回弹，回到释放状态
            }
        }
        break;

    case 2: // 按下状态，检测释放
        if (key_current_level != KEY0_PRESSED && key_current_level != KEY1_PRESSED)
        {
            key_debounce_tick = TIM1_Count;
            key_state = 3; // 进入释放消抖
        }
        break;

    case 3: // 释放消抖，稳定后复位
        if ((unsigned char)(TIM1_Count - key_debounce_tick) >= DEBOUNCE_MS)
        {
            key_state = 0; // 回到释放状态
        }
        break;

    default:
        key_state = 0;
        break;
    }
    return Ret_Val;
}
/*-------------------------------------------------
* 函数名：EEPROMread
* 功能：  读EEPROM数据
* 输入：  EEAddr需读取数据的地址
* 输出：  ReEEPROMread对应地址读出的数据
--------------------------------------------------*/
unsigned char EEPROMread(unsigned char EEAddr)
{
    unsigned char ReEEPROMread;
    while (GIE) // 等待GIE为0
    {
        GIE = 0; // 读数据必须关闭中断
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

    return ReEEPROMread;
}
/*-------------------------------------------------
 * 函数名：Unlock_Flash
 * 功能：  进行FLASH/EEDATA操作时，解锁FLASH/EEDATA的时序不能被打断。
 *		   程序中要将此段用汇编指令处理防止被优化
 * 输入：  无
 * 输出：  无
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
/*-------------------------------------------------
 * 函数名：EEPROMwrite
 * 功能：  写数据到EEPROM
 * 输入：  EEAddr为需要写入数据的地址，Data为需要写入的数据
 * 输出：  无
 --------------------------------------------------*/
void EEPROMwrite(unsigned char EEAddr, unsigned char Data)
{
    while (GIE) // 等待GIE为0
    {
        GIE = 0; // 写数据必须关闭中断
        NOP();
        NOP();
    }
    EEADRL = EEAddr; // EEPROM的地址
    EEDATL = Data;   // EEPROM的数据

    CFGS = 0;
    EEPGD = 0;
    WREN = 1; // 写使能
    EEIF = 0;

    Unlock_Flash(); // Flash 解锁时序不能修改
    NOP();
    NOP();
    NOP();
    NOP();

    // int timeout = 0;       // 超时计数器（防止永久阻塞）
    //    while(WR && timeout < 1000) // 等待WR=0，超时1000次（约10ms）
    //    {
    //        timeout++;
    //        NOP();                  // 短暂延时，降低CPU占用
    //    }

    while (WR)
        ; // 等待EEPROM写入完成
    WREN = 0;
    GIE = 1;
}
/*-------------------------------------------------
 * 函数名：TIM2_INITIAL
 * 功能：  初始化TIM2
 * 输入：  无
 * 输出：  无
 */
void main(void)
{
    POWER_INITIAL();
    TIM2_INITIAL();
    TIM1_INITIAL();

    串口初始化参数
    EEPROMwrite(0x13, 0x55); // 0x55写入地址0x13
    EEReadData = EEPROMread(0x13);
    UART_INITIAL(); // 使能串口，目前来看必须在write弄完了之后再开启中断，否则会无法解锁

    // 呼吸灯初始化参数
    TempH1 = TIM2CCR1H;
    TempL1 = TIM2CCR1L;
    ComValue1 = TempH1 * 256 + TempL1;
    T1 = 2 * (TempH1 * 256 + TempL1) - 1;
    Compare_Flag = 0;
    static unsigned char last_breath_tick = 0; // 呼吸更新时间戳

    串口初始化参数
    UR1TCF = 0; // 延迟一会，等待串口ok再读取
    UR1DATAL = EEReadData;

    while (1)
    {
        // 主循环中按键扫描部分
        unsigned char key_val = Key_Scan_NonBlock();
        if (key_val == KEY0_PRESSED)
        {
            breath_mode = KEY0_PRESSED;
            breath_Flag1 = 1; // 按键0=变亮（方向设为1）
        }
        else if (key_val == KEY1_PRESSED)
        {
            breath_mode = KEY1_PRESSED;
            breath_Flag1 = 0; // 按键1=变暗（方向设为0）
        }

        // 2. 呼吸模式：基于时间戳更新，无阻塞（修改后）
        if (breath_mode == KEY0_PRESSED) // 按键0=变亮
        {
            // 每50ms更新一次占空比，去掉breath_Flag1==1的多余条件
            if ((unsigned char)(TIM1_Count - last_breath_tick) >= 50)
            {
                last_breath_tick = TIM1_Count; // 用独立时间戳，避免修改TIM1_Count
                if (ComValue1 < T1)
                {
                    ComValue1 += 10;
                    if (ComValue1 > T1)
                        ComValue1 = T1;
                }
                else
                {
                    breath_mode = 0; // 到最暗，暂停
                }
                // 更新PA5（TIM2_CH1）占空比
                TIM2CCR1H = ComValue1 / 256;
                TIM2CCR1L = ComValue1 % 256;
            }
        }
        else if (breath_mode == KEY1_PRESSED) // 按键1=变暗
        {
            // 每50ms更新一次占空比，去掉breath_Flag1==0的多余条件
            if ((unsigned char)(TIM1_Count - last_breath_tick) >= 50)
            {
                last_breath_tick = TIM1_Count; // 用独立时间戳，避免干扰按键消抖
                if (ComValue1 > 0)
                {
                    ComValue1 -= 10;
                    if (ComValue1 < 0)
                        ComValue1 = 0;
                }
                else
                {
                    breath_mode = 0; // 到最亮，暂停
                }
                // 更新PA5（TIM2_CH1）占空比
                TIM2CCR1H = ComValue1 / 256;
                TIM2CCR1L = ComValue1 % 256;
            }
        }

        // 暂停模式：无操作，维持当前占空比
    }
}