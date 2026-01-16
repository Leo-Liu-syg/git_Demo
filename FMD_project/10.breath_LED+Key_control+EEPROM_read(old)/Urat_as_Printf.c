
#include	"SYSCFG.h"
#include 	"FT64F0AX.h"
/*-------------------------------------------------
 * 函数名：UART_SendString
 * 功能：  串口发送字符串
 * 输入：  str - 要发送的字符串指针
 * 输出：  无
 --------------------------------------------------*/
 
void UART_SendByte(unsigned char data)
{
    // ??????????????
    while(!UR1TXEF)
    {
        asm("nop");
    }
    UR1DATAL = data;            // ��???????????????
}

void UART_SendString(const unsigned char *str)
{
    while(*str)
    {
        UART_SendByte(*str++);
    }
}

/*-------------------------------------------------
 * 函数名：UART_SendHex
 * 功能：  串口发送16进制字节
 * 输入：  hex - 要发送的16进制数
 * 输出：  无
 --------------------------------------------------*/
void UART_SendHex(unsigned char hex)
{
    unsigned char temp;
    
    // 发送高4位
    temp = (hex >> 4) & 0x0F;
    if(temp < 10)
        UART_SendByte(temp + '0');
    else
        UART_SendByte(temp - 10 + 'A');
    
    // 发送低4位
    temp = hex & 0x0F;
    if(temp < 10)
        UART_SendByte(temp + '0');
    else
        UART_SendByte(temp - 10 + 'A');
}

/*-------------------------------------------------
 * 函数名：UART_SendDecimal
 * 功能：  串口发送十进制数（0-255）
 * 输入：  value - 要发送的十进制数
 * 输出：  无
 --------------------------------------------------*/
void UART_SendDecimal(unsigned char value)
{
    unsigned char hundred, ten, one;
    
    if(value >= 100)
    {
        hundred = value / 100;
        ten = (value % 100) / 10;
        one = value % 10;
        
        UART_SendByte(hundred + '0');
        UART_SendByte(ten + '0');
        UART_SendByte(one + '0');
    }
    else if(value >= 10)
    {
        ten = value / 10;
        one = value % 10;
        
        UART_SendByte(ten + '0');
        UART_SendByte(one + '0');
    }
    else
    {
        UART_SendByte(value + '0');
    }
}