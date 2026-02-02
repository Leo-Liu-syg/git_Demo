#include "SYSCFG.h"
#include "FT64F0AX.h"

//----------------EEPROM+URAT-------------------------------
/*-------------------------------------------------
* 函数名：EEPROMread
* 功能：  读EEPROM数据
* 输入：  EEAddr需读取数据的地址
* 输出：  ReEEPROMread对应地址读出的数据
--------------------------------------------------*/
unsigned char EEPROMread(unsigned char EEAddr)
{
	unsigned char ReEEPROMread;
	GIE = 0; // 读数据必须关闭中断
	NOP();
	NOP();
	if (GIE == 1)
	{
		GIE = 0;
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
	GIE = 1;
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
	MOVLW 0x03;
	MOVWF _BSREG;
	MOVLW 0x55;
	MOVWF _EECON2;
	MOVLW 0xAA;
	MOVWF _EECON2;
	BSF _EECON1, 1;
	NOP;
	NOP;
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

	GIE = 0;
	NOP();
	NOP();
	// 等待 2 个 NOP 的中断响应延时，再次判断 GIE 是否被清零
	if (GIE == 1)
	{
		GIE = 0;
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

	WR = 1;
	NOP();
	NOP();
	while (WR)
		; // 等待EEPROM写入时序的完成//
	EEIF = 1;
	WREN = 0;
	GIE = 1;
}
