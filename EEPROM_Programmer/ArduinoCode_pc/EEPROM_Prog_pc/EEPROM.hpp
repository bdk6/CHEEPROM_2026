//
//
//
//
//
//
#include <stdint.h>


bool EEPROM_setBaseAddress(uint16_t base);
uint16_t EEPROM_getBaseAddress();
bool EEPROM_setPower(bool on);
bool EEPROM_getPower();
bool EEPROM_setPageMode(bool page);
bool EEPROM_getPageMode();
bool EEPROM_isBlank();
bool EEPROM_setChipSize(uint16_t sz);
uint16_t EEPROM_getChipSize();

uint8_t EEPROM_readByte(uint16_t addr);
bool EEPROM_writeByte(uint16_t addr, uint8_t data);
bool EEPROM_writeRecord(uint16_t addr, uint8_t count, uint8_t* record);
