#include "hw_gpio.h"
#include "app_led.h"
#include "SYSCFG.h"

// 全局变量定义（对应原有TIM1_Count、Key_Mode，添加前缀g_提高辨识度）
unsigned int g_LED_TimerCount = 0;
unsigned char g_LED_CurrentMode = 0;
unsigned char g_LED_Intermediate = 0; // 模式切换中间变量（对应原有Intermediate）

// LED应用层初始化
void APP_LED_Init(void)
{
    g_LED_TimerCount = 0;
    g_LED_CurrentMode = LED_MODE_1;
    g_LED_Intermediate = 0;
    HW_LED_Write(LED_ON); // 初始状态LED点亮
}

// 模式1继续呼吸，模式2停止呼吸
void APP_LED_Update(void)
{
    switch(g_LED_CurrentMode)
    {
        case LED_MODE_1: // 模式1继续呼吸
        PCKEN |= 0B00000010;//使能定时器1
            break;
        
        case LED_MODE_2: // 模式2停止呼吸          
        PCKEN = PCKEN & ~(1<<1); // 0b00000110 原本的值 目标值0b00000100 关闭定时器1.
            break;
        default: // 默认模式1，防止异常
            g_LED_CurrentMode = LED_MODE_1;
            break;
    }
}

// LED模式切换（复用原有按键中的模式切换逻辑）
void APP_LED_SwitchMode(void)
{
    if(g_LED_CurrentMode < LED_MODE_MAX)
    {
        g_LED_Intermediate += 1;
        g_LED_CurrentMode = g_LED_Intermediate;
    }
    else
    {
        g_LED_Intermediate = 0;
        g_LED_CurrentMode = 0;
    }
    g_LED_TimerCount = 0; // 切换模式后重置计数，避免闪烁不连贯
}