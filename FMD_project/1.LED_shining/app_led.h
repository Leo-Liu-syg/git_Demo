#ifndef _APP_LED_H_
#define _APP_LED_H_

// 宏定义：LED闪烁模式参数（对应原有代码的4种模式）
#define LED_MODE_1    0   // 闪烁周期：2000（1000亮/1000灭）
#define LED_MODE_2    1   // 闪烁周期：4000（2000亮/2000灭）
#define LED_MODE_3    2   // 闪烁周期：1000（500亮/500灭）
#define LED_MODE_4    3   // 闪烁周期：200（100亮/100灭）
#define LED_MODE_MAX  3   // 最大模式编号

// 全局变量声明（需在app_led.c中定义）
extern unsigned int g_LED_TimerCount;  // LED闪烁计数（对应原有TIM1_Count）
extern unsigned char g_LED_CurrentMode; // LED当前闪烁模式（对应原有Key_Mode）

// 函数原型声明
void APP_LED_Init(void);  // LED应用层初始化（初始化计数和模式）
void APP_LED_Update(void); // LED闪烁逻辑更新（在中断中调用，更新计数和亮灭）
void APP_LED_SwitchMode(void); // LED模式切换（按键触发时调用）

#endif // _APP_LED_H_