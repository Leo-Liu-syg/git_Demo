#ifndef	_EEPROM_H_
#define	_EEPROM_H_

unsigned char EEPROM_Read(unsigned char addr);
void Unlock_Flash();
void EEPROM_Write(unsigned char addr, unsigned char data);

#endif