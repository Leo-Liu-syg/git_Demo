#ifndef __IR_NEC_H
#define __IR_NEC_H

#define u8  unsigned char
#define u16 unsigned int
#define u32 unsigned long

// 统一外部变量声明，和main.c里的定义匹配
extern volatile u8  ir_parse_ok;
extern volatile u8  ir_key_code;
extern volatile u8  ir_key_value;  // 保存最终键值
extern volatile u8  ir_key_ready;  // 键值有效标志

void IR_NEC_Init(void);
void IR_Edge_Process(void);
void IR_NEC_Parse(void);

#endif