// Project: TEST_FT64F0AX_USART.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description: 定时器1计时1秒，串口打印"power on：%ds"
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "URAT_INITIAL.h"
#include "HW_TIMER1.h"
#include <stdio.h>

//***********************定义与变量****************************
#define uchar unsigned char
#define UART_TX_BUF_SIZE 64

// 系统占用变量（勿删）
volatile char W_TMP @0x70;
volatile char BSR_TMP @0x71;

// 新增：定时器计时变量（1ms中断计数→1秒）
volatile uchar timer1_ms_cnt = 0;  // 1ms中断计数器
volatile bit timer1_1s_flag = 0;   // 1秒标志位（1=已到1秒）
int score = 0;                     // 计时变量（秒数）

// UART发送缓冲区（保留，适配无缓冲发送）
uchar uart_tx_buf[UART_TX_BUF_SIZE];
volatile uchar tx_buf_head = 0;
volatile uchar tx_buf_tail = 0;
volatile uchar tx_buf_empty = 1;

//发送相关变量
volatile uchar Send_flag = 0;

// 函数原型（统一格式）
void user_isr(void);
void uart_send_char(uchar c);
int fputc(int ch, FILE *f); 

//===========================================================
// UART发送单个字符（无缓冲）
//===========================================================
void uart_send_char(uchar c)
{
    while (!UR1TXEF);         // 等待发送缓冲区为空
    UR1DATAL = c;             // 写入发送寄存器
}
int fputc(int ch, FILE *f)  
{
    uart_send_char((uchar)ch);
    return ch;
}
//int fputc(int ch, void *f)
//{
//    uart_send_char((uchar)ch);
//    return ch;
//}//printf

//===========================================================
// 总中断服务函数
//===========================================================
void interrupt ISR(void)
{
#asm
    NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
    NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
    NOP;NOP;NOP;NOP;NOP;NOP;
#endasm
    user_isr();  // 调用用户中断处理
}

//===========================================================
// 用户中断处理（关键：新增TIMER1中断处理）
//===========================================================
void user_isr(void)
{
    // 1. TIMER1更新中断（1ms触发一次）
    if (TIM1SR1 & 0x01)  // 检查TIMER1中断标志
    {
        HW_TIMER1_Clear_ITFlag(); // 清除中断标志（调用封装函数）
        timer1_ms_cnt++;
        // 1ms计数→1秒（1000次1ms=1秒）
        if (timer1_ms_cnt >= 999)
        {
            timer1_ms_cnt = 0;
            timer1_1s_flag = 1;  // 置1秒标志
            Send_flag = 1;
            
        }
       
       
           
    }

    // 2. UART发送完成中断（保留，未启用）
    if (UR1TCEN && UR1TCF)
    {
       ////uart_send_char(0xaa); 
//        UR1TCF = 0;
//         if(Send_flag == 1) 
//        {
//            UR1DATAL = 0xaa; // 回传字符
//            Send_flag = 0; // 复位标志，等待下一次接收
//        }
//        // 缓冲发送逻辑（未启用，注释保留）
//        /*if(!tx_buf_empty)
//        {
//            UR1DATAL = uart_tx_buf[tx_buf_tail];
//            tx_buf_tail = (tx_buf_tail + 1) % UART_TX_BUF_SIZE;
//            tx_buf_empty = (tx_buf_head == tx_buf_tail) ? 1 : 0;
//        }*/
    }
}
// 系统初始化（时钟+IO+外设）
void POWER_INITIAL(void)
{
    // 1. 配置8MHz内部时钟（关键：先配置时钟）
    OSCCON = 0B01110001;  // 内部16MHz，1:1分频
    
    // 2. 关闭全局中断（初始化期间）
    INTCON = 0;
    
    // 3. IO口初始化（PA6=UART_TX输出，PA7=UART_RX输入）
    PORTA = 0B00000000;
    PORTB = 0B00000000;
    PORTC = 0B00000000;
    
    TRISA = 0B10000000;  // PA7=输入（UART_RX），PA6=输出（UART_TX）
    TRISB = 0B00000000;  // PB口全部输出（可选，用于测试）
    TRISC = 0B00000000;
    
    // 4. 禁用上下拉+模拟功能（数字IO）
    WPUA = 0; WPUB = 0; WPUC = 0;
    WPDA = 0; WPDB = 0; WPDC = 0;
    ANSELA = 0;  // 所有IO设为数字模式
}


void main(void)
{
    // 1. 初始化顺序：先系统时钟→再外设（关键修复）
    POWER_INITIAL();    // 系统时钟（16MHz）+ IO初始化
    UART_INITIAL();     // UART初始化（9600波特率）
    HW_TIMER1_Init();   // TIMER1初始化（1ms中断）
    UR1TCEN = 1;
    // 2. 开启全局中断（最后开启，确保外设已初始化）
    INTCON = 0B11000000;  // 全局中断使能 + 外设中断使能
    
    // 3. 初始打印（确认串口正常）
    printf("test\r\n");
    //uart_send_char(0xaa);
    
    // 4. 主循环（1秒打印一次）
    while (1)
    {
        //if (timer1_1s_flag == 1)  // 检查1秒标志
//        {
//            timer1_1s_flag = 0;   // 清除标志
//            score++;              // 秒数自增
//            uart_send_char(0xaa);
//           // UR1DATAL = 0xaa;
//            //printf("power on：%d\r\n", score);  // 打印指定格式
//        }
    }
}