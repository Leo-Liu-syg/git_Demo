#ifndef _HW_TIMER1_H_
#define _HW_TIMER1_H_

// 函数原型声明
void HW_TIMER1_Init(void);  // TIMER1全局初始化（含中断使能）
void HW_TIMER1_Clear_ITFlag(void); // 清除TIMER1更新中断标志位

#endif // _HW_TIMER1_H_