
///////////////////////////////////////////////////////////////////////
///  @file flash_rw.ino
///  @copy Copyright 2023 William R Cooke
///  @brief Demonstrates Winbond W25Q32 SPI flash
///////////////////////////////////////////////////////////////////////

#include <stdint.h>

#define CS    9
#define CLK   10
#define MOSI  11
#define MISO  12


// SPI mode 0 (or 3) D OUt on falling edge, in on rising edge
// MSB first, CS must rise after byte write for any write 
// instruction to proceed


uint8_t write_byte(uint8_t b)
{
  Serial.print("b:");
  Serial.print(b, HEX);
  Serial.print(":");
  
  uint8_t rtn = 0;
//  digitalWrite(CLK, 0);
//  digitalWrite(CS, 0);
  for(int bit = 0; bit < 8; bit++)
  {
    rtn <<= 1;
    if ( (b & 0x80) != 0)
    {
      digitalWrite(MOSI, 1);
    }
    else
    {
      digitalWrite(MOSI, 0);
    }
    digitalWrite(CLK, 1);
    delay(1);
    digitalWrite(CLK, 0);
    b <<= 1;
    //delay(1);

    uint8_t rd = digitalRead(MISO);
    Serial.print(rd);
    if (rd != 0)
    {
      rtn |= 0x01;
    }
    b <<= 1;
  }
 // digitalWrite(CS, 1);
  Serial.print(":");
  Serial.println(rtn, HEX);

  return rtn;
}



void setup() {
  Serial.begin(9600);

  pinMode(CS, OUTPUT);
  digitalWrite(CS, 1);
  pinMode(CLK, OUTPUT);
  digitalWrite(CLK, 1);
  pinMode(MOSI, OUTPUT);
  digitalWrite(MOSI, 0);
  pinMode(MISO, INPUT);
  delay(2000);

  // Release power down
  digitalWrite(CS,0);
  write_byte(0xab);
  write_byte(0);
  write_byte(0);
  write_byte(0);
  uint8_t response = write_byte(0);
  digitalWrite(CS, 1);
  Serial.print("rel pwr down: ");
  Serial.println(response, HEX);
  
  

}

void loop() {
 // digitalWrite(CS, 1);
 // digitalWrite(MOSI, 1);
 // digitalWrite(CLK, 1);
  
  static int counter = 0;
  //Serial.print("Serial output:");
  //Serial.print(counter++);
  //Serial.println();

  uint8_t response;
  digitalWrite(CS, 0);
  write_byte(0x90);  // manuf / device id
  write_byte(0);
  write_byte(0);
  write_byte(0);
  response = write_byte(0);
  Serial.println(response, HEX);
  response = write_byte(0);
  Serial.println(response, HEX);
  digitalWrite(CS, 1);
  
  delay(1000);

}
