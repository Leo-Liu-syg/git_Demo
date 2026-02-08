#ifndef __DS1302_H
#define __DS1302_H

#include "SYSCFG.h"
#include "FT61F0AX.h"

// 1. 引脚宏定义（依据：手册2.3.3 TRISB寄存器、2.3.6 PORTB寄存器）
#define DS1302_SCLK_PIN    0   // PB0
#define DS1302_DATA_PIN    1   // PB1
#define DS1302_RST_PIN     2   // PB2

// 2. 时间变量（独立定义，无结构体）
extern unsigned char ds1302_year;   // 0-99（2000-2099）
extern unsigned char ds1302_month;  // 1-12
extern unsigned char ds1302_day;    // 1-31
extern unsigned char ds1302_hour;   // 0-23
extern unsigned char ds1302_minute; // 0-59
extern unsigned char ds1302_second; // 0-59
extern unsigned char ds1302_week;   // 1-7（1=周日）

// 3. 函数声明
void DS1302_Init(void);                  // 初始化
void DS1302_WriteTime(void);             // 写入时间
void DS1302_ReadTime(void);              // 读取时间

#endif