#ifndef _APP_KEY_H_
#define _APP_KEY_H_

// 宏定义：按键消抖参数
#define KEY_DELAY_20MS    20  // 按键消抖所需20ms计数阈值

// 全局变量声明（消抖计数器，对应原有Delay_20ms_Count）
extern volatile unsigned char g_Key_DelayCount;

// 函数原型声明
void APP_KEY_Init(void);  // 按键应用层初始化
void APP_KEY_Process(void); // 按键逻辑处理（在主循环中调用）

#endif // _APP_KEY_H_