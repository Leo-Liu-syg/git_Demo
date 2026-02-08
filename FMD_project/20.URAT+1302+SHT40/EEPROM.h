#ifndef __EEPROM_H
#define __EEPROM_H

#include "SYSCFG.h"
#include "FT61F0AX.h"

unsigned char EEPROMread(unsigned char EEAddr);
void Unlock_Flash();
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);

#endif