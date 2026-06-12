
#include <Arduino.h>   // JUST FOR SERIAL debugging

#include "EEPROM.hpp"
#include "Interface.hpp"



#define CHIP_8K      8192
#define CHIP_32K    32768


static uint16_t baseAddress = 0;
bool powerOn = false;
static uint16_t chipSize = CHIP_8K;   // Valid sizes are 8192, 32768
static bool pageMode = false;         // True if page mode selected (32K only)

///////////////////////////////////////////////////////////////////
///  @fn EEPROM_setBaseAddress
///  @brief Set base address of EEPROM if it fits into 64K space
///  @param[in] base   The start address EEPROM will be addressed
///  @return true if the setting was made
///////////////////////////////////////////////////////////////////
bool  EEPROM_setBaseAddress(uint16_t base)
{
  bool rtn = true;
  // Make sure this results in a 64K address range
  uint32_t lastAddress = (long) base + chipSize;
  if(lastAddress > 65535L)
  {
    rtn = false;
  }
  else
  {
    baseAddress = base;
  }
}


///////////////////////////////////////////////////////////////////
///  @fn EEPROM_getBaseAddress
///  @brief Get the current base address setting
///  @return The current base address setting
///////////////////////////////////////////////////////////////////
uint16_t EEPROM_getBaseAddress()
{
  return baseAddress;
}


////////////////////////////////////////////////////////////
///  @fn EEPROM_setPower
///  @brief Turn EEPROM power on or off
///  @param[in] on If true, turn on power, else turn it off
///  @return true if success.
////////////////////////////////////////////////////////////
bool EEPROM_setPower(bool on)
{
  //Serial.println("Setting power to ");
  //Serial.println(on);
  bool rtn = true;
  interface_power(on);
  powerOn = on;
  return rtn;
}


//////////////////////////////////////////////////////////////////
///  @fn EEPROM_getPower
///  @brief Get status of EEPROM power
///  @return true if EEPROM is powered
//////////////////////////////////////////////////////////////////
bool EEPROM_getPower()
{
  return powerOn;
}


//////////////////////////////////////////////////////////////////
///  @fn EEPROM_setPageMode
///  @brief Turn page mode on or off 
///  @param[in] page   true for page mode, false for byte mode
///  @return true if command succesful
//////////////////////////////////////////////////////////////////
bool EEPROM_setPageMode(bool page)
{
  bool rtn = false;
  if(page && chipSize == CHIP_32K)
  {
    pageMode = true;
    rtn = true;
  }
  else if(page)   // Not 32k chip
  {
    pageMode = false;
    rtn = false;         // Tell shell we couldn't set page mode
  }
  else
  {
    pageMode = false;
    rtn = true;
  }

  return rtn;
}


/////////////////////////////////////////////////////////////////
///  @fn EEPROM_isBlank
///  @brief Check the EEPROM for all 0xff indicating erased
///  @return true if EEPROM is blank
/////////////////////////////////////////////////////////////////
bool EEPROM_isBlank()
{
  bool rtn = true;
  for(uint16_t addr = 0; addr < chipSize; addr++)
  {
    uint8_t res = interface_readByte(addr);
    if(res != 0xff)
    {
      rtn = false;
    }
  }
  return rtn;
}


////////////////////////////////////////////////////////////////
///  @fn EEPROM_setChipSize
///  @brief Selects either 8K x 8 or 32K x 8 EEPROM
///  @param[in] sz  Size in bytes: 8192 or 32768
///  @return true if setting was made
////////////////////////////////////////////////////////////////
bool EEPROM_setChipSize(uint16_t sz)
{
  bool rtn = false;
  if(sz == CHIP_32K)
  {
    chipSize = CHIP_32K;
    rtn = true;
  }
  else if(sz == CHIP_8K)
  {
    chipSize = CHIP_8K;
    rtn = true;
  }
  // Any other combination is an error

  return rtn;
}


////////////////////////////////////////////////////////////////
///  @fn EEPROM_getChipSize
///  @brief Gets the currently selected EEPROM size
///  @return Size of currently selected EEPROM in bytes.
////////////////////////////////////////////////////////////////
uint16_t EEPROM_getChipSize()
{
  return chipSize;
}


uint8_t EEPROM_readByte(uint16_t addr)
{
  return interface_readByte(addr);
}


bool EEPROM_writeByte(uint16_t addr, uint8_t data)
{
  return interface_writeByte(addr, data);
}


////////////////////////////////////////////////////////////////
///  @fn EEPROM_writeRecord
///  @brief Writes a group of consecutive bytes to EEPROM
///  @param[in] addr  The starting address of data writes.
///  @param[in] count Number of bytes to write.
///  @param[in] record An array holding the count and data bytes.
///  @return true if write was succesful
////////////////////////////////////////////////////////////////
bool EEPROM_writeRecord(uint16_t addr, uint8_t count, uint8_t* record)
{
  // TODO:  Add page mode
  
  bool rtn = true;
  if(record == (uint8_t*) 0 )
  {
    Serial.println("writeRecord failed from null ptr");
    rtn = false;
  }
  else
  {
    Serial.println("Writing data...");
    for(int i = 0; i < count; i++)
    {
      uint8_t data = record[i];
      interface_writeByte(addr + i, data);
    }
    // Now verifty
    for(int i = 0; i < count; i++)
    {
      uint8_t data = record[i];
      uint8_t eepData = interface_readByte(addr + i);
      //Serial.print(addr + i); Serial.print(": "); Serial.print(data); Serial.print(" : "); Serial.println(eepData);
      
      if(interface_readByte(addr + i) != data)
      {
        rtn = false;
      }
    }
  }


  return rtn;
}
