#include "hw_gpio.h"
#include "app_led.h"

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

// LED闪烁逻辑更新（核心：复用原有中断中的LED逻辑）
void APP_LED_Update(void)
{
    switch(g_LED_CurrentMode)
    {
        case LED_MODE_1: // 模式1：1000亮/1000灭
            if(g_LED_TimerCount < 1000)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_ON);
            }
            else if(g_LED_TimerCount >= 1000 && g_LED_TimerCount < 2000)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_OFF);
            }
            else if(g_LED_TimerCount >= 2000)
            {
                g_LED_TimerCount = 0;
                HW_LED_Write(LED_ON);
            }
            break;
        
        case LED_MODE_2: // 模式2：2000亮/2000灭
            if(g_LED_TimerCount < 2000)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_ON);
            }
            else if(g_LED_TimerCount >= 2000 && g_LED_TimerCount < 4000)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_OFF);
            }
            else if(g_LED_TimerCount >= 4000)
            {
                g_LED_TimerCount = 0;
                HW_LED_Write(LED_ON);
            }
            break;
        
        case LED_MODE_3: // 模式3：500亮/500灭
            if(g_LED_TimerCount < 500)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_ON);
            }
            else if(g_LED_TimerCount >= 500 && g_LED_TimerCount < 1000)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_OFF);
            }
            else if(g_LED_TimerCount >= 1000)
            {
                g_LED_TimerCount = 0;
                HW_LED_Write(LED_ON);
            }
            break;
        
        case LED_MODE_4: // 模式4：100亮/100灭
            if(g_LED_TimerCount < 100)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_ON);
            }
            else if(g_LED_TimerCount >= 100 && g_LED_TimerCount < 200)
            {
                g_LED_TimerCount++;
                HW_LED_Write(LED_OFF);
            }
            else if(g_LED_TimerCount >= 200)
            {
                g_LED_TimerCount = 0;
                HW_LED_Write(LED_ON);
            }
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