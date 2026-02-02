#ifndef __KEY_NONBLOCK_H
#define __KEY_NONBLOCK_H

#define KEY_PIN     PA0  // 按键引脚（PA0）
#define KEY_PRESSED 0    // 按键按下电平（上拉输入，按下为低）
#define KEY_RELEASED 1   // 按键释放电平
#define DEBOUNCE_MS 20   // 按键消抖时间（20ms）

unsigned char Key_Scan_NonBlock(void);

#endif // __KEY_H