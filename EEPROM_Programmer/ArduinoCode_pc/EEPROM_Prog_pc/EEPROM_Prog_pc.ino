//  EEPROM_Prog.ino
//  NEW--Read and Write Atmel AT28C64 and AT28C256 EEPROM chips From special Linux program
//  copyright 2022 William R Cooke
//  
//  Uses hardware consisting of Arduino Nano and two 74HC595 shift registers
//  Connects to *** special Linux program ***
//  Programs Intel Hex files to EEPROM
//  Reads data from EEPROM and sends as Intel Hex file
//
//  Supports following commands:
//    B xxxx                    -- Set base address of EEPROM to xxxx (hex value)
//    R                         -- Reads entire contents of EEPROM and sends as Hex file
//    :xxxxxxxxxxx...           -- Programs Intel Hex data line
//    E                         -- End programming of Hex file



#include "Interface.hpp"
#include "Shell.hpp"




void setup() 
{
  interface_initialize();
//  shell_initialize();
  Serial.println("back to setup");

  //Serial.begin(115200);
  Serial.begin(9600);
  

}

bool LED_on = false;

int count = 0;

void loop() 
{
  //0
  Serial.print("Welcome: ");
  shell();

  
//  Serial.println(count);
//  count++;
//  delay(1000);


////interface_lightLED(true);
//interface_power(true);
//delay(1000);
////interface_lightLED(false);
//interface_power(false);
//delay(1000);

  
//  shell();
//  interface_Power(LED_on);
//  LED_on = !LED_on;
//  interface_lightLED(LED_on);
//  delay(250);
  

}
