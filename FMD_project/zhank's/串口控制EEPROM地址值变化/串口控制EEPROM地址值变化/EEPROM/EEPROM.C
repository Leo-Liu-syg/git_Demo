//********************************************************* 
/* 文件名：TEST_EEPROM_UART.c
* 功能：   FT61F0Ax EEPROM读取并通过串口打印
* IC:      FT61F0A5 TSSOP20
* 说明：   上电读取EEPROM 0x13地址的值并通过串口打印
*          通过串口接收电脑指令操作EEPROM数据
* 
*                FT61F0A5  TSSOP20
*              -------------------
* NC----------|1(PA5)   	(PA4)20|-----------NC     
* TXIO--------|2(PA6)   	(PA3)19|-----------NC 
* RXIO--------|3(PA7)   	(PA2)18|-----------NC
* NC----------|4(PC0)   	(PA1)17|-----------NC
* NC----------|5(PC1)		(PA0)16|-----------NC	
* NC----------|6(PB7)		(PB0)15|-----------NC
* GND---------|7(GND)		(PB1)14|-----------NC
* NC----------|8(PB6)		(PB2)13|-----------NC
* VDD---------|9(VDD)		(PB3)12|-----------NC
* NC----------|10(PB5)		(PB4)11|-----------NC
*				-------------------
*/ 
//*********************************************************
#include	"SYSCFG.h"
#include 	"FT64F0AX.h"

//***********************宏定义****************************
#define		uchar		unsigned char
#define     uint        unsigned int

// EEPROM操作地址
#define EEPROM_ADDR 0x13

// 串口指令定义
#define CMD_INCREMENT 0xAA    // 值+1
#define CMD_DECREMENT 0xBB    // 值-1

// NOP宏定义
#define NOP() asm("nop")

volatile uchar EEReadData;
volatile uchar UART_RxData = 0;
volatile bit UART_RxFlag = 0;

/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
    OSCCON = 0B01110001;        // 系统时钟选择为内部振荡器16MHz,分频比为1:1
    INTCON = 0;                 // 禁止所有中断
    
    PORTA = 0B00000000;
    PORTB = 0B00000000;
    PORTC = 0B00000000;
    
    WPUA = 0B00000000;          // 弱上拉的开关，0-关，1-开		
    WPUB = 0B00000000;
    WPUC = 0B00000000;	

    WPDA = 0B00000000;          // 弱下拉的开关，0-关，1-开
    WPDB = 0B00000000;
    WPDC = 0B00000000;
    
    // 重要：设置PA6为输出（TX），PA7为输入（RX）
    TRISA = 0B10000000;         // PA7=1(输入RX), PA6=0(输出TX)，其他输出
    
    PSRC0 = 0B11111111;         // 源电流设置最大
    PSRC1 = 0B11111111;
    PSRC2 = 0B00001111;

    PSINK0 = 0B11111111;        // 灌电流设置最大
    PSINK1 = 0B11111111;
    PSINK2 = 0B00000011;

    ANSELA = 0B00000000;        // 设置对应的IO为数字IO
}

/*-------------------------------------------------
 * 函数名：DelayUs
 * 功能：  短延时函数
 * 输入：  Time延时时间长度 延时时长Time Us
 * 输出：  无
 --------------------------------------------------*/
void DelayUs(uchar Time)
{
    uchar a;
    for(a = 0; a < Time; a++)
    {
        asm("nop");  // 使用内联汇编
    } 
}

/*-------------------------------------------------
 * 函数名：DelayMs
 * 功能：  短延时函数
 * 输入：  Time延时时间长度 延时时长Time ms
 * 输出：  无
 --------------------------------------------------*/
void DelayMs(uchar Time)
{
    uchar a, b;
    for(a = 0; a < Time; a++)
    {
        for(b = 0; b < 5; b++)
        {
            DelayUs(197);
        }
    }
}

/*-------------------------------------------------
 * 函数名：UART_INITIAL
 * 功能：  初始化串口
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void UART_INITIAL(void)
{
    PCKEN |= 0B00100000;        // 使能UART1模块时钟
    
    // 先配置波特率
    UR1DLL = 104;               // 波特率=Fmaster/(16*{URxDLH,URxDLL})=9600 @16MHz
    UR1DLH = 0;
    
    UR1LCR = 0B10000011;        // 使能LCR访问，8位数据长度，1位停止位，无奇偶校验位
    UR1LCR = 0B00000011;        // 关闭LCR访问
    
    UR1MCR = 0B00011000;        // 使能发送和接收接口
    
    UR1IER = 0B00000001;        // 使能接收中断 (UR1RXNE)
    UR1TCF = 1;                 // 清除发送完成标志
    
    // 清除可能存在的接收标志
    UR1RXNEF = 0;
}

/*-------------------------------------------------
 * 函数名：UART_SendByte
 * 功能：  串口发送一个字节（轮询方式）
 * 输入：  data - 要发送的数据
 * 输出：  无
 --------------------------------------------------*/
void UART_SendByte(uchar data)
{
    // 等待发送缓冲区空
    while(!UR1TXEF)
    {
        asm("nop");
    }
    UR1DATAL = data;            // 写入数据到发送寄存器
}

/*-------------------------------------------------
 * 函数名：UART_SendString
 * 功能：  串口发送字符串
 * 输入：  str - 要发送的字符串指针
 * 输出：  无
 --------------------------------------------------*/
void UART_SendString(const uchar *str)
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
void UART_SendHex(uchar hex)
{
    uchar temp;
    
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
void UART_SendDecimal(uchar value)
{
    uchar hundred, ten, one;
    
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

/*-------------------------------------------------
 * 函数名：EEPROMread
 * 功能：  读EEPROM数据
 * 输入：  EEAddr需读取数据的地址
 * 输出：  ReEEPROMread对应地址读出的数据
 --------------------------------------------------*/
uchar EEPROMread(uchar EEAddr)
{
    uchar ReEEPROMread;
    while(GIE)                  // 等待GIE为0
    {
        GIE = 0;                // 读数据必须关闭中断
        asm("nop");               
        asm("nop");            
    }				
    EEADRL = EEAddr;
    
    CFGS = 0;
    EEPGD = 0;
    RD = 1;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    ReEEPROMread = EEDATL;
    
    GIE = 1;  // 恢复中断
    return ReEEPROMread;
}

/*-------------------------------------------------
 * 函数名：Unlock_Flash
 * 功能：  进行FLASH/EEDATA操作时，解锁FLASH/EEDATA的时序不能被打断。
 *         程序中要将此段用汇编指令处理防止被优化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void Unlock_Flash()
{
    asm("MOVLW 0x03");
    asm("MOVWF _BSREG");
    asm("MOVLW 0x55");
    asm("MOVWF _EECON2");
    asm("MOVLW 0xAA");
    asm("MOVWF _EECON2");
    asm("BSF _EECON1,1");   // WR=1;
    asm("nop");
    asm("nop");
}

/*-------------------------------------------------
 * 函数名：EEPROMwrite
 * 功能：  写数据到EEPROM
 * 输入：  EEAddr为需要写入数据的地址，Data为需要写入的数据
 * 输出：  无
 --------------------------------------------------*/
void EEPROMwrite(uchar EEAddr, uchar Data)
{
    while(GIE)                  // 等待GIE为0
    {
        GIE = 0;                // 写数据必须关闭中断
        asm("nop");               
        asm("nop");            
    }				
    EEADRL = EEAddr;            // EEPROM的地址
    EEDATL = Data;              // EEPROM的数据
     
    CFGS = 0;
    EEPGD = 0;
    WREN = 1;                   // 写使能
    EEIF = 0;
    
    Unlock_Flash();             // Flash 解锁时序不能修改
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    
    while(WR);                  // 等待EEPROM写入完成
    WREN = 0;
    GIE = 1;
}

/*-------------------------------------------------
 * 函数名：UpdateEEPROMDisplay
 * 功能：  更新EEPROM显示
 * 输入：  value - EEPROM的值
 * 输出：  无
 --------------------------------------------------*/
void UpdateEEPROMDisplay(uchar value)
{
    UART_SendString("\r\nEEPROM Updated: 0x");
    UART_SendHex(value);
    UART_SendString(" (Decimal: ");
    UART_SendDecimal(value);
    UART_SendString(")\r\n");
}

/*-------------------------------------------------
 * 函数名：UART_InterruptHandler
 * 功能：  串口中断处理函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void UART_InterruptHandler(void) interrupt 11
{
    // 检查接收数据就绪标志 (UR1RXNEF)
    if(UR1RXNEF)  // 接收数据就绪
    {
        // 读取接收到的数据
        UART_RxData = UR1DATAL;
        UART_RxFlag = 1;  // 设置接收标志
        
        // 清除接收标志（可选，有些芯片读取数据后自动清除）
        UR1RXNEF = 0;  // 尝试清除标志
    }
    
    // 检查发送完成标志 (UR1TCF)
    if(UR1TCF)  // 发送完成
    {
        UR1TCF = 1;  // 清除发送标志
    }
}

/*-------------------------------------------------
 * 函数名：ProcessUARTCommand
 * 功能：  处理串口接收到的命令
 * 输入：  command - 接收到的命令
 * 输出：  无
 --------------------------------------------------*/
void ProcessUARTCommand(uchar command)
{
    uchar eeprom_value;
    
    // 先确认收到了数据
    UART_SendString("\r\nReceived Command: 0x");
    UART_SendHex(command);
    UART_SendString("\r\n");
    
    // 读取当前EEPROM值
    eeprom_value = EEPROMread(EEPROM_ADDR);
    
    switch(command)
    {
        case CMD_INCREMENT:  // 0xAA: EEPROM值+1
            UART_SendString("CMD: Increment EEPROM value\r\n");
            eeprom_value++;
            EEPROMwrite(EEPROM_ADDR, eeprom_value);
            UpdateEEPROMDisplay(eeprom_value);
            break;
            
        case CMD_DECREMENT:  // 0xBB: EEPROM值-1
            UART_SendString("CMD: Decrement EEPROM value\r\n");
            eeprom_value--;
            EEPROMwrite(EEPROM_ADDR, eeprom_value);
            UpdateEEPROMDisplay(eeprom_value);
            break;
            
        default:  // 其他值: 直接设置EEPROM值为接收到的值
            UART_SendString("CMD: Set EEPROM value to 0x");
            UART_SendHex(command);
            UART_SendString("\r\n");
            EEPROMwrite(EEPROM_ADDR, command);
            UpdateEEPROMDisplay(command);
            break;
    }
}

/*-------------------------------------------------
 * 函数名：ShowHelpInfo
 * 功能：  显示帮助信息
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void ShowHelpInfo(void)
{
    UART_SendString("\r\n=== EEPROM Control Commands ===\r\n");
    UART_SendString("Send 0xAA: Increment EEPROM value by 1\r\n");
    UART_SendString("Send 0xBB: Decrement EEPROM value by 1\r\n");
    UART_SendString("Send 0x00-0xFF: Set EEPROM to this value\r\n");
    UART_SendString("===================================\r\n\r\n");
}

/*-------------------------------------------------
 * 函数名：CheckUARTReceive
 * 功能：  轮询方式检查UART接收（备用方案）
 * 输入：  无
 * 输出：  接收到的数据，0表示没有数据
 --------------------------------------------------*/
uchar CheckUARTReceive(void)
{
    if(UR1RXNEF)  // 检查接收标志
    {
        uchar data = UR1DATAL;
        UR1RXNEF = 0;  // 清除标志
        return data;
    }
    return 0;
}

/*-------------------------------------------------
 * 函数名：main
 * 功能：  主函数 
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void main(void)
{
    uchar eeprom_value;
    uchar rx_data;
    
    POWER_INITIAL();            // 系统初始化
    UART_INITIAL();             // 串口初始化
    DelayMs(100);               // 延时等待系统稳定
    
    // 全局中断使能
    GIE = 1;
    PEIE = 1;                   // 使能外设中断
    
    // 如果需要，先写入测试数据到EEPROM地址0x13（只执行一次）
    // 注意：每次上电都会写入0x60，所以值不会变化
    EEPROMwrite(EEPROM_ADDR, 0x60); 
    DelayMs(10);  // 等待EEPROM写入完成
    
    // 读取EEPROM地址0x13的值
    eeprom_value = EEPROMread(EEPROM_ADDR);
    EEReadData = eeprom_value;  // 保存到全局变量
    
    // 通过串口打印启动信息和EEPROM值
    UART_SendString("\r\n======== FT61F0Ax EEPROM Test ========\r\n");
    
    UART_SendString("EEPROM Address 0x13 Value: 0x");
    UART_SendHex(eeprom_value);
    
    UART_SendString(" (Decimal: ");
    UART_SendDecimal(eeprom_value);
    UART_SendString(")\r\n");
    
    // 显示帮助信息
    ShowHelpInfo();
    
    while(1)
    {
        // 方法1：使用中断方式
        if(UART_RxFlag)
        {
            UART_RxFlag = 0;  // 清除接收标志
            
            // 处理接收到的命令
            ProcessUARTCommand(UART_RxData);
        }
        
        // 方法2：轮询方式（备用，如果中断不工作）
        rx_data = CheckUARTReceive();
        if(rx_data != 0)
        {
            // 处理接收到的命令
            ProcessUARTCommand(rx_data);
        }
        
        // 主循环，可以添加其他功能
        DelayMs(10);
    }
}