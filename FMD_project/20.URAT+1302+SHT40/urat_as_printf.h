#ifndef _URAT_AS_PRINTF_H_
#define _URAT_AS_PRINTF_H_

void UART_SendByte(unsigned char data);
/*-------------------------------------------------
 * 函数名：UART_SendString
 * 功能：  串口发送字符串
 * 输入：  str - 要发送的字符串指针
 * 输出：  无
 --------------------------------------------------*/
void UART_SendString(const unsigned char *str);
/*-------------------------------------------------
 * 函数名：UART_SendHex
 * 功能：  串口发送16进制字节
 * 输入：  hex - 要发送的16进制数
 * 输出：  无
 --------------------------------------------------*/
void UART_SendHex(unsigned char hex);
/*-------------------------------------------------
 * 函数名：UART_SendDecimal
 * 功能：  串口发送十进制数（0-255）
 * 输入：  value - 要发送的十进制数
 * 输出：  无
 --------------------------------------------------*/
void UART_SendDecimal(unsigned char value);
#endif // URAT_AS_PRINTF