#include "hw_gpio.h"
#include "app_led.h"
#include "app_key.h"

// 全局变量定义：按键消抖计数器（volatile修饰，中断中修改）
volatile unsigned char g_Key_DelayCount = 0;

// 按键应用层初始化
void APP_KEY_Init(void)
{
    g_Key_DelayCount = 0;
}

// 按键逻辑处理（核心：复用原有主循环中的按键逻辑）
void APP_KEY_Process(void)
{
    // 第一步：检测到按键按下
    if(HW_KEY_Read() == 1)
    {
        // 第二步：重置延时计数器，开始累计20ms
        g_Key_DelayCount = 0;
        
        // 第三步：等待累计20ms（消抖，非阻塞）
        if(g_Key_DelayCount < KEY_DELAY_20MS)
        {return;}
        
        // 第四步：延时后再次确认按键是否仍按下（有效按键判断）
        if(HW_KEY_Read() == 1) 
        {
            // 第五步：触发LED模式切换
            APP_LED_SwitchMode();
            
            // 第六步：等待按键释放，避免重复触发
            while(HW_KEY_Read() == 1);
        }
    }
}
