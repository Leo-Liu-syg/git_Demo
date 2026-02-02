// Project: TEST_64F0Ax_EEPROM.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description: 此演示程序位64F0Ax_EEPROM的演示程序.
//		   		把0x55写入地址0x13，再读出该值
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-4        修改系统时钟为8MHz，使能LVR
//
//                FT64F0A5  TSSOP20
//              -------------------
// NC----------|1(PA5)   	(PA4)20|-----------NC
// NC----------|2(PA6)   	(PA3)19|-----------NC
// NC----------|3(PA7)   	(PA2)18|-----------NC
// NC----------|4(PC0)   	(PA1)17|-----------NC
// NC----------|5(PC1)		(PA0)16|-----------NC
// NC----------|6(PB7)		(PB0)15|-----------NC
// GND---------|7(GND)		(PB1)14|-----------NC
// NC----------|8(PB6)		(PB2)13|-----------NC
// VDD---------|9(VDD)		(PB3)12|-----------NC
// NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
//
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "URAT_INITIAL.h"
#include "Delay.h"
//***********************宏定义****************************
// #define		uchar	unsigned char

// Variable definition
volatile char W_TMP @0x70;	 // 系统占用不可以删除和修改
volatile char BSR_TMP @0x71; // 系统占用不可以删除和修改
volatile unsigned char receivedata = 0;
volatile unsigned char sendaddress = 0;
volatile unsigned char senddata = 0;
volatile unsigned char EEReadData = 0xAA;
unsigned char send_flag = 0;
unsigned char init_send_done = 0;

void user_isr(); // 用户中断程序，不可删除
unsigned char EEPROMread(unsigned char EEAddr);
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);

//===========================================================
// Funtion name：interrupt ISR
// parameters：无
// returned value：无
//===========================================================
void interrupt ISR(void)
{
#asm;			// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
#endasm;		// 系统设置不可以删除和修改
	user_isr(); // 调用用户中断函数
}
void user_isr() // 调用用户中断函数
{
	if (UR1RXNE && UR1RXNEF) // 串口接收from电脑
	{
		UR1RXNEF = 0;
		EEReadData = UR1DATAL;
		if (EEReadData == 0xAA)
		{
			EEReadData = EEPROMread(0x13); // 读原来的ROM
								  
			EEPROMwrite(0x13, EEReadData+1);		   // 写入ROM
			
		}
		else if (EEReadData == 0xBB)
		{
			EEReadData = EEPROMread(0x13);						   // 自减
			EEPROMwrite(0x13, EEReadData-1);		   // 写入ROM
			 // ROM存入readdata
		}
        else if (EEReadData >= 0xff)
		{
			EEPROMwrite(0x13, 0x03);
		}
		else
		{           
			EEPROMwrite(0x13, EEReadData);
		}
		send_flag = 1;
	}

	if (UR1TCEN && UR1TCF) // 串口发送回电脑
	{
		UR1TCF = 0;
		if (send_flag == 1)
		{
			senddata = EEPROMread(0x13);
			UR1DATAL = senddata;
			send_flag = 0;
		}
	}
}
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 系统时钟选择为内部振荡器16MHz,分频比为2:1
	INTCON = 0;			 // 禁止所有中断

	PORTA = 0B00000000; // PA7 芯片rx
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA = 0B00000000; // 弱上拉的开关，0-关，1-开
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // 弱下拉的开关，0-关，1-开
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B10000000; // 输入输出设置，0-输出，1-输入
	TRISB = 0B00000000;
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // 源电流设置最大
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // 灌电流设置最大
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // 设置对应的IO为数字IO
}
/*-------------------------------------------------
* 函数名：EEPROMread
* 功能：  读EEPROM数据
* 输入：  EEAddr需读取数据的地址
* 输出：  ReEEPROMread对应地址读出的数据
--------------------------------------------------*/
unsigned char EEPROMread(unsigned char EEAddr)
{
	unsigned char ReEEPROMread;
	while (GIE) // 等待GIE为0
	{
		GIE = 0; // 读数据必须关闭中断
		NOP();
		NOP();
	}
	EEADRL = EEAddr;

	CFGS = 0;
	EEPGD = 0;
	RD = 1;
	NOP();
	NOP();
	NOP();
	NOP();
	ReEEPROMread = EEDATL;

	return ReEEPROMread;
}
/*-------------------------------------------------
 * 函数名：Unlock_Flash
 * 功能：  进行FLASH/EEDATA操作时，解锁FLASH/EEDATA的时序不能被打断。
 *		   程序中要将此段用汇编指令处理防止被优化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void Unlock_Flash()
{
	#asm
		MOVLW    0x03        ; 
		MOVWF    _BSREG      ; 
		MOVLW    0x55        ; 
		MOVWF    _EECON2     ; 
		MOVLW    0xAA        ; 
		MOVWF    _EECON2     ;
		BSF      _EECON1, 1  ; 
		NOP                  ; 
		NOP                  ; 
	#endasm
//	#asm
//		MOVLW 0x03 ;
//        MOVWF _BSREG
//		MOVLW 0x55 MOVWF _EECON2 &
//		0x7F MOVLW 0xAA 
//        MOVWF _EECON2 & 0x7F 
//        BSF _EECON1 & 0x7F, 1 // WR=1;
//		NOP 
//        NOP
//	#endasm
}
/*-------------------------------------------------
 * 函数名：EEPROMwrite
 * 功能：  写数据到EEPROM
 * 输入：  EEAddr为需要写入数据的地址，Data为需要写入的数据
 * 输出：  无
 --------------------------------------------------*/
void EEPROMwrite(unsigned char EEAddr, unsigned char Data)
{
	while (GIE) // 等待GIE为0
	{
		GIE = 0; // 写数据必须关闭中断
		NOP();
		NOP();
	}
	EEADRL = EEAddr; // EEPROM的地址
	EEDATL = Data;	 // EEPROM的数据

	CFGS = 0;
	EEPGD = 0;
	WREN = 1; // 写使能
	EEIF = 0;

	Unlock_Flash(); // Flash 解锁时序不能修改
	NOP();
	NOP();
	NOP();
	NOP();

	// int timeout = 0;       // 超时计数器（防止永久阻塞）
	//    while(WR && timeout < 1000) // 等待WR=0，超时1000次（约10ms）
	//    {
	//        timeout++;
	//        NOP();                  // 短暂延时，降低CPU占用
	//    }

	while (WR)
		; // 等待EEPROM写入完成
	WREN = 0;
	GIE = 1;
}
/*-------------------------------------------------
 * 函数名：main
 * 功能：	 主函数
 * 输入：	 无
 * 输出： 	 无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL();			   // 系统初始化
	EEPROMwrite(0x13, 0x55);	   // 0x55写入地址0x13
	EEReadData = EEPROMread(0x13); // 芯片上电的时候把EEPROM 0x13房间里的内容通过串口显示出来
	// EEReadData=EEPROMread(0x13);//0x13地址EEPROM值，存到EEReadData里

	UART_INITIAL(); // 使能串口，目前来看必须在write弄完了之后再开启中断，否则会无法解锁
	DelayMs(100);	// 延迟一会，等待串口ok
	UR1TCF = 0;
	UR1DATAL = EEReadData;
	
	while (1)
	{
		
		DelayMs(10);
	}
}