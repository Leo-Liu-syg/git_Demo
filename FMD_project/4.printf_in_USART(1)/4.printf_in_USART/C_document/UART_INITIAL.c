#include	"SYSCFG.h"
#include 	"FT64F0AX.h" 
 
void UART_INITIAL(void)
{
    PCKEN |= 0B00100000;			// 使能UART1模块时钟
    UR1IER = 0B00100001;			// 使能发送完成中断+接收数据中断
    UR1LCR = 0B00000001;			// 8位数据，1位停止位，无奇偶校验（8N1）
    UR1MCR = 0B00011000;			// 使能发送和接收接口
       
    UR1DLL = 52;					// 8MHz时钟 → 9600波特率（计算公式：UR1DLL = Fosc/(16*Baud) = 8e6/(16*9600) ≈ 52）
    UR1DLH = 0;  
    INTCON = 0B11000000;			// 使能全局中断+外设中断
}