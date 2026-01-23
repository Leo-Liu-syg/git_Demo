#ifndef	_EEPROM_H_
#define	_EEPROM_H_

unsigned char EEPROMread(unsigned char EEAddr);
void Unlock_Flash();
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);

#endif