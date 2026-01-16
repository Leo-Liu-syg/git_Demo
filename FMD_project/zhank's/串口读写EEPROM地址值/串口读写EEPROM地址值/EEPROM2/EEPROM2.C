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
#define CMD_WRITE_EEPROM 0xAA    // 写EEPROM
#define CMD_READ_EEPROM  0xBB    // 读EEPROM

// NOP宏定义
#define NOP() asm("nop")

// 全局变量
volatile uchar EEReadData;
volatile uchar UART_RxBuffer[10];
volatile uchar UART_RxIndex = 0;
volatile bit UART_RxComplete = 0;

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
    
    // 设置PA6为输出（TX），PA7为输入（RX）
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
    
    // 配置线路控制寄存器
    UR1LCR = 0B10000011;        // 使能LCR访问，8位数据长度，1位停止位，无奇偶校验位
    UR1LCR = 0B00000011;        // 关闭LCR访问
    
    UR1MCR = 0B00011000;        // 使能发送和接收接口
    
    // 重要：设置UR1IER为接收中断使能
    UR1IER = 0B00000001;        // 使能接收中断 (UR1RXNE)
    
    // 清除所有可能的中断标志
    UR1TCF = 1;                 // 清除发送完成标志
    UR1LSR = 0;                 // 清除线路状态寄存器中的所有标志
}

/*-------------------------------------------------
 * 函数名：UART_SendByte
 * 功能：  串口发送一个字节（轮询方式）
 * 输入：  data - 要发送的数据
 * 输出：  无
 --------------------------------------------------*/
void UART_SendByte(uchar data)
{
    // 等待发送缓冲区空（检查UR1LSR的第5位UR1TXEF）
    while(!(UR1LSR & 0x20))  // UR1TXEF = UR1LSR.5
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
 * 函数名：UART_InterruptHandler
 * 功能：  串口中断处理函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void UART_InterruptHandler(void) interrupt 11
{
    // 检查接收数据就绪标志 (UR1LSR的第0位UR1RXNEF)
    if(UR1LSR & 0x01)  // 接收数据就绪 (UR1RXNEF)
    {
        uchar rx_data;
        
        // 读取接收到的数据
        rx_data = UR1DATAL;
        
        // 保存数据到缓冲区
        if(UART_RxIndex < 10)
        {
            UART_RxBuffer[UART_RxIndex] = rx_data;
            UART_RxIndex++;
            
            // 检查是否收到完整的数据包
            // 先检查第一个字节是什么命令
            if(UART_RxIndex == 1)
            {
                // 刚收到第一个字节，检查是什么命令
                if(rx_data == CMD_WRITE_EEPROM)
                {
                    // 写命令需要3个字节，继续等待
                }
                else if(rx_data == CMD_READ_EEPROM)
                {
                    // 读命令需要2个字节，继续等待
                }
                else
                {
                    // 未知命令，清空缓冲区
                    UART_RxIndex = 0;
                }
            }
            else if(UART_RxIndex >= 3)
            {
                // 至少收到3个字节，检查是否是有效的写命令
                if(UART_RxBuffer[0] == CMD_WRITE_EEPROM)
                {
                    UART_RxComplete = 1;
                }
            }
            else if(UART_RxIndex >= 2)
            {
                // 至少收到2个字节，检查是否是有效的读命令
                if(UART_RxBuffer[0] == CMD_READ_EEPROM)
                {
                    UART_RxComplete = 1;
                }
            }
        }
        else
        {
            // 缓冲区满，重置
            UART_RxIndex = 0;
        }
        
        // 注意：读取数据后标志会自动清除，不需要手动清除
    }
    
    // 检查发送完成标志 (UR1TC的第0位UR1TCF)
    if(UR1TC & 0x01)  // 发送完成
    {
        UR1TC = 1;  // 清除发送标志
    }
}

/*-------------------------------------------------
 * 函数名：CheckUARTCommand
 * 功能：  检查并处理UART命令
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void CheckUARTCommand(void)
{
    uchar i;
    
    if(UART_RxComplete)
    {
        UART_SendString("\r\n[Packet Received] ");
        for(i = 0; i < UART_RxIndex; i++)
        {
            UART_SendString("0x");
            UART_SendHex(UART_RxBuffer[i]);
            UART_SendString(" ");
        }
        UART_SendString("\r\n");
        
        // 检查命令类型
        if(UART_RxBuffer[0] == CMD_WRITE_EEPROM && UART_RxIndex >= 3)
        {
            UART_SendString("[Write Command] ");
            UART_SendString("Addr=0x");
            UART_SendHex(UART_RxBuffer[1]);
            UART_SendString(", Data=0x");
            UART_SendHex(UART_RxBuffer[2]);
            UART_SendString("\r\n");
            
            // 写入EEPROM
            EEPROMwrite(UART_RxBuffer[1], UART_RxBuffer[2]);
            
            UART_SendString("[Write Success]\r\n");
        }
        else if(UART_RxBuffer[0] == CMD_READ_EEPROM && UART_RxIndex >= 2)
        {
            uchar read_data;
            
            UART_SendString("[Read Command] ");
            UART_SendString("Addr=0x");
            UART_SendHex(UART_RxBuffer[1]);
            UART_SendString("\r\n");
            
            // 读取EEPROM
            read_data = EEPROMread(UART_RxBuffer[1]);
            
            // 返回数据给电脑
            UART_SendString("[Read Data: 0x");
            UART_SendHex(read_data);
            UART_SendString("]\r\n");
            
            // 也发送原始数据字节
            UART_SendByte(read_data);
        }
        else
        {
            UART_SendString("[Invalid Command]\r\n");
        }
        
        // 重置接收状态
        UART_RxIndex = 0;
        UART_RxComplete = 0;
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
    UART_SendString("\r\n======== FT61F0Ax EEPROM Control ========\r\n");
    UART_SendString("Command Format (HEX mode):\r\n");
    UART_SendString("  1. Write EEPROM: AA 20 55\r\n");
    UART_SendString("     Example: Send 'AA 20 55' to write 0x55 to address 0x20\r\n");
    UART_SendString("  2. Read EEPROM:  BB 20\r\n");
    UART_SendString("     Example: Send 'BB 20' to read value from address 0x20\r\n");
    UART_SendString("Send as HEX bytes (no 0x prefix, with spaces)\r\n");
    UART_SendString("==========================================\r\n\r\n");
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
    
    POWER_INITIAL();            // 系统初始化
    UART_INITIAL();             // 串口初始化
    DelayMs(100);               // 延时等待系统稳定
    
    // 全局中断使能
    GIE = 1;
    PEIE = 1;                   // 使能外设中断
    
    // 读取并显示EEPROM 0x13的初始值
    eeprom_value = EEPROMread(0x13);
    EEReadData = eeprom_value;  // 保存到全局变量
    
    // 通过串口打印启动信息
    UART_SendString("\r\n======== FT61F0Ax EEPROM Test ========\r\n");
    UART_SendString("System Initialized\r\n");
    UART_SendString("EEPROM Addr 0x13 = 0x");
    UART_SendHex(eeprom_value);
    UART_SendString(" (");
    UART_SendDecimal(eeprom_value);
    UART_SendString(")\r\n");
    
    // 显示帮助信息
    ShowHelpInfo();
    
    UART_SendString("[Ready for commands]\r\n");
    
    // 主循环
    while(1)
    {
        // 检查并处理UART命令
        CheckUARTCommand();
        
        // 主循环延时
        DelayMs(10);
    }
}