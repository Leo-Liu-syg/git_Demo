// Project: TEST_64F0Ax_TIM2_PWM.prj
// Device:  FT64F0AX
// 核心功能：PA0=提高亮度，PA1=降低亮度，PA5=LED（高电平点亮）
// 修复重点：PWM输出使能、按键检测、中断触发、EEPROM默认值
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "URAT_INITIAL.h"
#include "delay.h"

// *********************** 关键宏定义（适配硬件） ****************************
#define LED_PWM_PIN PA5       // LED连接PA5（TIM2_CH1）
#define KEY_UP_PIN  PA0       // 提高亮度按键（PA0）
#define KEY_DOWN_PIN PA1      // 降低亮度按键（PA1）
#define KEY_PRESSED 0         // 按键按下电平（上拉输入→低电平有效）
#define PWM_MAX     999       // PWM最大占空比（ARR=1000）
#define PWM_MIN     0         // PWM最小占空比
#define BRIGHT_STEP 50        // 亮度调节步长（越大调节越明显）
#define SAVE_DELAY_MS 1000    // 松开后保存延时（1000ms）
#define EEPROM_ADDR_H 0x14    // 亮度高8位保存地址
#define EEPROM_ADDR_L 0x15    // 亮度低8位保存地址
#define DEFAULT_BRIGHT 500    // 首次上电默认亮度（50%）

// *********************** 全局变量 ****************************
volatile char W_TMP @0x70;   // 系统占用，不可删除
volatile char BSR_TMP @0x71; // 系统占用，不可删除
unsigned int ComValue1 = DEFAULT_BRIGHT; // 当前亮度值（0~999）
volatile unsigned int Key_Scan_count = 0; // TIM1中断计数（1ms/次）
volatile unsigned int save_delay_cnt = 0; // 保存延时计数器
unsigned char key_up_flag = 0;  // 按键松开标志（1=需保存）
unsigned char key_down_flag = 0;

// 串口+EEPROM相关变量
volatile unsigned char EEReadData = 0xAA;
unsigned char send_flag = 0;

// 函数声明
void interrupt ISR(void);
void user_isr();
void GPIO_INIT(void);
void TIM1_INIT(void);  // 1ms中断计时
void TIM2_PWM_INIT(void); // PWM输出初始化
unsigned char Key_Read(void); // 简化按键读取（消抖）
unsigned char EEPROM_Read(unsigned char addr);
void EEPROM_Write(unsigned char addr, unsigned char data);
void Unlock_Flash();

//===========================================================
// 中断入口函数（系统固定）
//===========================================================
void interrupt ISR(void)
{
#asm
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
    NOP; NOP; NOP; NOP; NOP; NOP;
#endasm
    user_isr();
}

//===========================================================
// 用户中断处理（仅计时+保存，简化逻辑）
//===========================================================
void user_isr()
{
    if (T1UIF) // TIM1更新中断（1ms触发一次）
    {
        T1UIF = 1; // 写1清零标志位
        Key_Scan_count++;

        // 按键松开后延时1秒保存
        if (key_up_flag || key_down_flag)
        {
            save_delay_cnt++;
            if (save_delay_cnt >= SAVE_DELAY_MS)
            {
                // 保存当前亮度到EEPROM
                EEPROM_Write(EEPROM_ADDR_H, ComValue1 / 256);
                EEPROM_Write(EEPROM_ADDR_L, ComValue1 % 256);
                send_flag = 1; // 串口反馈
                save_delay_cnt = 0;
                key_up_flag = 0;
                key_down_flag = 0;
            }
        }
    }

    // 串口发送反馈（可选，用于调试）
    if (UR1TCEN && UR1TCF && send_flag)
    {
        UR1TCF = 0;
        UR1DATAL = 0x41; // 发送'A'表示保存成功
        send_flag = 0;
    }
}

/*-------------------------------------------------
 * 函数名：GPIO_INIT
 * 功能：  配置GPIO（LED输出、按键输入）
 --------------------------------------------------*/
void GPIO_INIT(void)
{
    // 初始化GPIO端口
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;

    // 按键配置（PA0/PA1：上拉输入）
    TRISA |= (1 << KEY_UP_PIN) | (1 << KEY_DOWN_PIN); // PA0/PA1输入
    WPUA |= (1 << KEY_UP_PIN) | (1 << KEY_DOWN_PIN);  // 开启弱上拉
    ANSELA &= ~((1 << KEY_UP_PIN) | (1 << KEY_DOWN_PIN)); // 数字IO

    // LED配置（PA5：PWM输出）
    TRISA &= ~(1 << LED_PWM_PIN); // PA5输出
    ANSELA &= ~(1 << LED_PWM_PIN); // 数字IO

    // 电流配置（最大驱动能力）
    PSRC0 = 0xFF;
    PSINK0 = 0xFF;
}

/*-------------------------------------------------
 * 函数名：TIM1_INIT
 * 功能：  初始化TIM1为1ms中断（用于计时）
 --------------------------------------------------*/
void TIM1_INIT(void)
{
    PCKEN |= (1 << 1); // 使能TIM1模块时钟（PCKEN bit1）
    TCKSRC |= 0x03;   // TIM1时钟源：HIRC*2（32MHz）
    TIM1ARRH = 0x7C;  // 自动重载值：0x7CFF = 31999
    TIM1ARRL = 0xFF;
    TIM1CR1 = 0x85;   // 允许自动装载，使能计数器（T1CEN=1）
    TIM1IER = 0x01;   // 允许TIM1更新中断
    INTCON = 0xC0;    // 使能全局中断（GIE=1）和外设中断（PEIE=1）
}

/*-------------------------------------------------
 * 函数名：TIM2_PWM_INIT
 * 功能：  初始化TIM2为PWM输出（高电平点亮LED）
 --------------------------------------------------*/
void TIM2_PWM_INIT(void)
{
    PCKEN |= (1 << 2); // 使能TIM2模块时钟（PCKEN bit2）
    TCKSRC |= 0x30;   // TIM2时钟源：HIRC*2（32MHz）
    TIM2ARRH = 0x03;  // PWM周期：0x03E8=1000（频率32kHz）
    TIM2ARRL = 0xE8;

    // 配置TIM2_CH1为PWM1模式，高电平有效
    TIM2CCMR1 = 0x6A; // T2OC1M=110（PWM1），T2OC1PE=1（预装载）
    TIM2CCER1 = 0x03; // CC1E=1（输出使能），CC1P=1（高电平有效）

    // 加载初始占空比
    TIM2CCR1H = ComValue1 / 256;
    TIM2CCR1L = ComValue1 % 256;

    TIM2CR1 = 0x85;   // 使能TIM2计数器（T2CEN=1）
}

/*-------------------------------------------------
 * 函数名：Key_Read
 * 功能：  简化按键读取（消抖20ms）
 * 返回值：0=无按键，1=提高键，2=降低键
 --------------------------------------------------*/
unsigned char Key_Read(void)
{
    static unsigned char key_up_last = 1;
    static unsigned char key_down_last = 1;
    static unsigned int key_up_cnt = 0;
    static unsigned int key_down_cnt = 0;
    unsigned char ret = 0;

    // 读取按键当前电平
    unsigned char key_up_now = (PORTA >> KEY_UP_PIN) & 0x01;
    unsigned char key_down_now = (PORTA >> KEY_DOWN_PIN) & 0x01;

    // 提高键消抖
    if (key_up_now != key_up_last)
    {
        key_up_cnt = 0;
        key_up_last = key_up_now;
    }
    else if (key_up_cnt < 20) // 消抖20ms
    {
        key_up_cnt++;
    }

    // 降低键消抖
    if (key_down_now != key_down_last)
    {
        key_down_cnt = 0;
        key_down_last = key_down_now;
    }
    else if (key_down_cnt < 20)
    {
        key_down_cnt++;
    }

    // 检测按键按下（消抖稳定后）
    if (key_up_cnt >= 20 && key_up_now == KEY_PRESSED)
    {
        ret = 1;
        key_up_cnt = 0; // 单次触发
    }
    else if (key_down_cnt >= 20 && key_down_now == KEY_PRESSED)
    {
        ret = 2;
        key_down_cnt = 0;
    }

    // 检测按键松开（设置保存标志）
    if (key_up_now == 1 && key_up_last == 0)
    {
        key_up_flag = 1;
        key_down_flag = 0;
        save_delay_cnt = 0;
    }
    else if (key_down_now == 1 && key_down_last == 0)
    {
        key_down_flag = 1;
        key_up_flag = 0;
        save_delay_cnt = 0;
    }

    return ret;
}

/*-------------------------------------------------
 * 函数名：EEPROM_Read
 * 功能：  读EEPROM（简化逻辑，确保中断开启）
 --------------------------------------------------*/
unsigned char EEPROM_Read(unsigned char addr)
{
    unsigned char data;
    GIE = 0; // 关闭中断
    EEADRL = addr;
    CFGS = 0;
    EEPGD = 0;
    RD = 1;
    NOP(); NOP(); NOP(); NOP();
    data = EEDATL;
    GIE = 1; // 开启中断
    return data;
}

/*-------------------------------------------------
 * 函数名：Unlock_Flash
 * 功能：  EEPROM写解锁（固定时序）
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
    NOP; NOP;
#endasm
}

/*-------------------------------------------------
 * 函数名：EEPROM_Write
 * 功能：  写EEPROM
 --------------------------------------------------*/
void EEPROM_Write(unsigned char addr, unsigned char data)
{
    GIE = 0;
    EEADRL = addr;
    EEDATL = data;
    CFGS = 0;
    EEPGD = 0;
    WREN = 1;
    Unlock_Flash();
    while (WR); // 等待写入完成
    WREN = 0;
    GIE = 1;
}

/*-------------------------------------------------
 * 函数名：main
 * 功能：  主函数（简化逻辑，确保每一步生效）
 --------------------------------------------------*/
void main(void)
{
    // 1. 初始化GPIO（先配置硬件，再初始化外设）
    GPIO_INIT();
    DelayMs(50); // 稳定延时

    // 2. 读取EEPROM保存的亮度值（首次上电用默认值）
    unsigned char bright_h = EEPROM_Read(EEPROM_ADDR_H);
    unsigned char bright_l = EEPROM_Read(EEPROM_ADDR_L);
    unsigned int saved_bright = (bright_h << 8) | bright_l;
    // 校验保存的亮度值是否有效（0~999）
    if (saved_bright >= PWM_MIN && saved_bright <= PWM_MAX)
    {
        ComValue1 = saved_bright;
    }
    else
    {
        ComValue1 = DEFAULT_BRIGHT; // 无效值用默认
    }

    // 3. 初始化PWM和中断
    TIM2_PWM_INIT();
    TIM1_INIT();
    UART_INITIAL(); // 串口初始化（可选，用于调试）

    // 主循环（仅按键检测和亮度更新）
    while (1)
    {
        unsigned char key_val = Key_Read();

        // 按键调节亮度
        if (key_val == 1) // 提高亮度
        {
            ComValue1 += BRIGHT_STEP;
            if (ComValue1 > PWM_MAX) ComValue1 = PWM_MAX;
            // 更新PWM占空比
            TIM2CCR1H = ComValue1 / 256;
            TIM2CCR1L = ComValue1 % 256;
        }
        else if (key_val == 2) // 降低亮度
        {
            if (ComValue1 < BRIGHT_STEP) ComValue1 = PWM_MIN;
            else ComValue1 -= BRIGHT_STEP;
            // 更新PWM占空比
            TIM2CCR1H = ComValue1 / 256;
            TIM2CCR1L = ComValue1 % 256;
        }

        DelayMs(10); // 降低CPU占用
    }
}