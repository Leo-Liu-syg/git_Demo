#ifndef _HW_GPIO_H_
#define _HW_GPIO_H_

// 宏定义：简化GPIO操作，提高可读性
#define KEY_PIN_PA4    4       // 按键对应引脚PA4
#define LED_PIN_PA5    5       // LED对应引脚PA5
#define LED_PIN_PA3    3       // LED对应引脚PA5


// 函数原型声明
void HW_GPIO_Init(void);      // GPIO全局初始化（PA4输入上拉、PA5输出）
unsigned char HW_KEY_Read(void); // 读取按键状态（1=按下，0=未按下）
void HW_LED_Write(unsigned char state); // 控制LED亮灭（传入LED_ON/LED_OFF）

#endif // _HW_GPIO_H_