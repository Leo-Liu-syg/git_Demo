#include "SYSCFG.h"
#include "FT64F0AX.h"

/*-------------------------------------------------
 * 函数名：EEPROM_Read
 * 功能：  读EEPROM（简化逻辑，确保中断开启）
 --------------------------------------------------*/
unsigned char EEPROM_Read(unsigned char addr)
{
    unsigned char data;
    GIE = 0; // 关闭中断
    EEADRL = addr;
    CFGS = 0;
    EEPGD = 0;
    RD = 1;
    NOP(); NOP(); NOP(); NOP();
    data = EEDATL;
    GIE = 1; // 开启中断
    return data;
}

/*-------------------------------------------------
 * 函数名：Unlock_Flash
 * 功能：  EEPROM写解锁（固定时序）
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
    NOP; NOP;
#endasm
}

/*-------------------------------------------------
 * 函数名：EEPROM_Write
 * 功能：  写EEPROM
 --------------------------------------------------*/
void EEPROM_Write(unsigned char addr, unsigned char data)
{
    GIE = 0;
    EEADRL = addr;
    EEDATL = data;
    CFGS = 0;
    EEPGD = 0;
    WREN = 1;
    Unlock_Flash();
    while (WR); // 等待写入完成
    WREN = 0;
    GIE = 1;
}
