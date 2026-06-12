//
//
//

#include <Arduino.h>
#include "Interface.hpp"

#define PIN_RCLK      2      // hc595 register clock       (PD2)
#define PIN_SRCLK     13     // hc595 shift register clock (PB5)
#define PIN_SER       11     // hc595 serial data          (PB3)

#define PIN_WE        5      // EEPROM write enable        (PD5)
#define PIN_OE        4      // EEPROM output enable       (PD4)
#define PIN_CS        3      // EEPROM chip select         (PD3)

#define PIN_VCC_CNTL1 8      // EEPROM Vcc control 1       (PB0)
#define PIN_VCC_CNTL2 9      // EEPROM Vcc control 2       (PB1)

#define PIN_LED_GRN   10     // Green LED control          (PB2)

// Data bus is on analog pins ADC0 (0) to ADC5 (5) and D6(6) and D7(7) (PC0-5), (PD6-7)



static void setOE(bool on);
static void setWE(bool on);
static void setCS(bool on);

static bool writeAddressBus(uint16_t addr);
static bool setDataOutput(bool out);
static bool writeDataBus(uint8_t byt);
static int16_t readDataBus();




bool interface_initialize()
{
  bool rtn = true;
  // Set all pin directions and starting state
  pinMode(PIN_RCLK, OUTPUT);
  digitalWrite(PIN_RCLK, LOW);
  pinMode(PIN_SRCLK, OUTPUT);
  digitalWrite(PIN_SRCLK, LOW);
  pinMode(PIN_SER, OUTPUT);
  digitalWrite(PIN_SER, LOW);
  pinMode(PIN_WE, OUTPUT);
  digitalWrite(PIN_WE, HIGH);              // Disable WE
  pinMode(PIN_OE, OUTPUT);
  digitalWrite(PIN_OE, HIGH);              // Disable OE
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);              // Disable CS
  
  pinMode(PIN_VCC_CNTL1, OUTPUT);           // Setup EEPROM Vcc pins and turn off Vcc
  digitalWrite(PIN_VCC_CNTL1, LOW);
  pinMode(PIN_VCC_CNTL2, OUTPUT);
  digitalWrite(PIN_VCC_CNTL2, LOW);

  writeAddressBus(0);                      // Clear address bus to 0 so no power to pins
  setDataOutput(false);                    // Set data pins to input for same reason

  pinMode(PIN_LED_GRN, OUTPUT);            // Ready to go, light green led
  digitalWrite(PIN_LED_GRN, HIGH);
  
  return rtn;
}


////////////////////////////////////////////////////////////////////////
///  @fn interface_writeByte
///  @brief  Reads a byte from EEPROM
///  @param[in] addr  The address to write to
///  @param[in] byt   The data to write to EEPROM
///  @return true if write is succesful
////////////////////////////////////////////////////////////////////////
bool interface_writeByte(uint16_t addr, uint8_t byt)
{
  bool rtn = true;
  setDataOutput(true);
  writeAddressBus(addr);
  writeDataBus(byt);
  setCS(false);
  setWE(false);
  setWE(true);
  setDataOutput(false);
  
  // Wait for write to finish
  uint8_t written = byt & 0x80;   // B7 inverts.  Only check it
  int16_t current;
  int tries = 0;
  do
  {
    setOE(false);
    current = readDataBus();
    setOE(true);
    if(current < 0)  // error reading
    {
      rtn = false;
      break;
    }
    if(tries++ > 30000)
    {
      rtn = false;
      Serial.println("write timeout");
      break;
    }
  }while(written != (uint8_t) (current & 0x80));
 
  return rtn;
}


int interface_readByte(uint16_t addr)
{
  int rtn = -1;
  setDataOutput(false);
  writeAddressBus(addr);
  setCS(false);
  setOE(false);
  rtn = readDataBus();
  setOE(true);
  setCS(true);

  return rtn;
}

static void setOE(bool on)
{
  if(on)
  {
    digitalWrite(PIN_OE, HIGH);
  }
  else
  {
    digitalWrite(PIN_OE, LOW);
  }
}


static void setWE(bool on)
{
  if(on)
  {
    digitalWrite(PIN_WE, HIGH);
  }
  else
  {
    digitalWrite(PIN_WE, LOW);
  }
}


static void setCS(bool on)
{
  if(on)
  {
    digitalWrite(PIN_CS, HIGH);
  }
  else
  {
    digitalWrite(PIN_CS, LOW);
  }
}


/////////////////////////////////////////////////////////////////////////
///  @fn interface_power
///  @brief Toggles power off/on and CS,WE,OE to safe states
///  @param[in] on true to turn power on, false to to turn it off
/////////////////////////////////////////////////////////////////////////
void interface_power(bool on)
{
  // The pins are set as high output on initialization.  Setting as inputs make them 
  // High Z so no power.  Setting as output drives them high, providing power to 
  // EEPROM and red power LED.
  
  if(on)
  {
    interface_lightLED(false);       // turn SAFE LED off
    setCS(true);                     // Control pins to deactive state
    setWE(true);
    setOE(true);
 //   writeAddressBus(0);              // Make sure all pins are 0 so no phantom power
    pinMode(PIN_VCC_CNTL1, OUTPUT);
    pinMode(PIN_VCC_CNTL2, OUTPUT);
    digitalWrite(PIN_VCC_CNTL1, HIGH);
    digitalWrite(PIN_VCC_CNTL2, HIGH);
  }
  else
  {
    writeAddressBus(0);              // Set all input pins to 0 to avoid phantom power
    setOE(false);
    setCS(false);
    setWE(false);
    pinMode(PIN_VCC_CNTL1, INPUT);
    pinMode(PIN_VCC_CNTL2, INPUT);
    interface_lightLED(true);       // turn SAFE LED on
  }
}

void interface_lightLED(bool on)
{
  bool rtn = false;
  if(on)
  {
    digitalWrite(PIN_LED_GRN, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED_GRN, LOW);
  }
  return rtn;
}


static bool writeAddressBus(uint16_t addr)
{
  bool rtn = true;
  // write 16 bits, MSB first
  for(int b = 0; b < 16; b++)
  {
    uint16_t bit = addr & 0x8000;
    if(bit != 0)
    {
      digitalWrite(PIN_SER, HIGH);
    }
    else
    {
      digitalWrite(PIN_SER, LOW);
    }
    // strobe the clock
    digitalWrite(PIN_SRCLK, HIGH);
    digitalWrite(PIN_SRCLK, LOW);
    addr <<= 1;       // get the next bit
  }

  // strobe the register clock
  digitalWrite(PIN_RCLK, HIGH);
  digitalWrite(PIN_RCLK, LOW);

  return rtn;
}

static bool setDataOutput(bool out)
{
  bool rtn = true;
  if(out)
  {
    for(int p = 14; p < 20; p++)
    {
      pinMode(p, OUTPUT);
    }
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
  }
  else
  {
    for(int p = 14; p < 20; p++)
    {
      pinMode(p, INPUT);
    }
    pinMode(6, INPUT);
    pinMode(7, INPUT);
  }

  return rtn;
}


static bool writeDataBus(uint8_t byt)
{
  bool rtn = true;
  setDataOutput(true);
  for(int p = 14; p < 20; p++)
  {
    if(byt & 0x01)
    {
      digitalWrite(p, HIGH);
    }
    else
    {
      digitalWrite(p, LOW);
    }
    byt >>= 1;
  }
  if(byt & 0x01)
  {
    digitalWrite(6, HIGH);
  }
  else
  {
    digitalWrite(6, LOW);
  }
  byt >>= 1;
  if(byt & 0x01)
  {
    digitalWrite(7, HIGH);
  }
  else
  {
    digitalWrite(7, LOW);
  }

  return rtn;
}


static int16_t readDataBus()
{
  int16_t rtn = -1;
  setDataOutput(false);
  uint16_t rd = PINC & 0x3f;
  uint16_t rd2 = PIND & 0xc0;
  rtn = rd | rd2;

  return rtn;
}
