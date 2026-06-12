
#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include <stdint.h>

bool interface_initialize();
void interface_power(bool on);
void interface_lightLED(bool on);

int interface_readByte(uint16_t addr);
bool interface_writeByte(uint16_t addr, uint8_t byt);

#endif
