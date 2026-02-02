#include "SYSCFG.h"
#include "FT61F0AX.h"
#include "hw_timer1.h"

// TIMER1全局初始化（复用原有TIM1_INITIAL逻辑）
void HW_TIMER1_Init(void)
{
    PCKEN |= 0B00000010;     // 使能TIMER1模块时钟
    CKOCON = 0B00100000;     // Timer1倍频时钟占空比调节位4ns延迟
    TCKSRC = 0B00000011;     // Timer1时钟源为HIRC的2倍频
    
    TIM1CR1 = 0B10000101;    // 允许自动装载，使能计数器
    TIM1IER = 0B00000001;    // 允许更新中断
       
    TIM1ARRH = 0x1F;         // 自动装载高8位
    TIM1ARRL = 0x3F;         // 自动装载低8位
    
    // 注：总中断使能保留在main.c中，避免模块跨权限操作系统全局中断
}

// 清除TIMER1更新中断标志位（封装T1UIF操作，提高可读性）
void HW_TIMER1_Clear_ITFlag(void)
{
    T1UIF = 1; // 复位中断标志位（芯片特有操作，保留原有逻辑）
}