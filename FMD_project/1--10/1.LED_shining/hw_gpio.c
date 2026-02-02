#include "SYSCFG.h"
#include "FT61F0AX.h"
#include "hw_gpio.h"

// GPIO全局初始化（整合原有PA4_Init、PA5_Init逻辑）
void HW_GPIO_Init(void)
{
    // PA4配置：输入模式 + 内部上拉
    TRISA |= (1 << KEY_PIN_PA4);  // 配置为输入
    WPUA |= (1 << KEY_PIN_PA4);   // 使能内部弱上拉
    
    // PA5配置：输出模式 + 初始熄灭
    TRISA &= ~(1 << LED_PIN_PA5); // 配置为输出
    HW_LED_Write(LED_OFF);        // 初始状态设置为熄灭
}

// 读取按键状态（复用原有PA4_Read_Key逻辑）
unsigned char HW_KEY_Read(void)
{
    if ( (PORTA & (1 << KEY_PIN_PA4)) == 0 )
    {
        return 1;  // 按键按下
    } 
    else
    {
        return 0;  // 按键未按下
    }
}

// 控制LED亮灭（封装PA5电平写入）
void HW_LED_Write(unsigned char state)
{
    if (state == LED_ON)
    {
        PORTA &= ~(1 << LED_PIN_PA5); // PA5置0，LED点亮
    }
    else
    {
        PORTA |= (1 << LED_PIN_PA5);  // PA5置1，LED熄灭
    }
}