#include "SYSCFG.h"
#include "FT61F0AX.h"
#include "hw_timer1.h"

// TIMER1全局初始化（复用原有TIM1_INITIAL逻辑）
void HW_TIMER1_Init(void)
{

    PCKEN |= 0B00000010;
    CKOCON = 0B00100000;
    TCKSRC = 0B00000011;

    TIM1CR1 = 0B10000101;

    TIM1IER = 0B00000001;

    TIM1ARRH = 0x1F;
    TIM1ARRL = 0x3F;

    INTCON = 0B11000000;
}
