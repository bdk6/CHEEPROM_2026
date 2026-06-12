//
//
//
//

#include <Arduino.h>
#include <ctype.h>
#include "Shell.hpp"
#include "EEPROM.hpp"

//#include "Interface.hpp"

#define XON     0x11              // DC1 / XON character / CTRL Q
#define XOFF    0x13              // DC3 / XOFF character / CTRL S



// Intel HEX can have up to 255 bytes, 2 hex chars per byte, plus up to 13 overhead chars
//#define MAX_LINE    (255*2+13)
#define MAX_LINE (16*2+13)


// Size of output records
#define OUTPUT_RECORD_SIZE       (16)

static char lineBuffer[MAX_LINE];      // Line input storage
static int nextCharIdx = 0;            // Where to take the next character from
static uint8_t codeBytes[64]; // 255];         // place to put parsed hex code
static uint8_t codeByteCount = 0;      // how many are in the array
static uint16_t codeByteAddress = 0;   // Address for first byte in array

static uint8_t hexLine[44];            // :+ADDR+TYPE+COUNT+DATA(16)+CS+term

static int getLine();
static bool parseCommand();
static bool skipToEOL(char lookfor = '\r');
static bool parseIHEX();
static bool sendIHEX();
static int nextChar();
static void makeHexLine(uint8_t count, uint16_t addr, uint8_t rec, uint8_t* data, char* output);
static bool asciiToHex(int numChars, uint16_t* result);
static void showMenu();



bool shell_initialize()
{
  bool rtn = true;
  Serial.begin(115200);

  for(int i = 0; i < MAX_LINE; i++)
  {
    lineBuffer[i] = 0;
  }
//  while(1)
//  {
//    Serial.println("Hello, PC!");
//    delay(1000);
//  }

  delay(2000);
  Serial.println("Waiting...");
  // wait for key
  while(!Serial.available());

  while(Serial.available())
  {
    Serial.read();
  }

  Serial.println("EEPROM programmer for AT28C256 and AT28C64(x)");
  Serial.println("Version 0.1 Feb 19 2022, by BDK6 (aka wrc)");
  showMenu();
  //Serial.println("Back ...");
  
  
  return rtn;
}


/////////////////////////////////////////////////////////////////////
///  @fn shell
///  @brief Command interpreter for programmer
///  @return  0 on success, error code on error
/////////////////////////////////////////////////////////////////////
int shell()
{
  int rtn = 0;
  Serial.print("EEP>");
  rtn = getLine();
  //Serial.println(rtn);
  //Serial.print((char)XOFF);
  //Serial.print("EEP >");
  parseCommand();

  return rtn;
}


////////////////////////////////////////////////////////////////////
///  @fn getLine
///  @brief Reads a line from Serial port into line buffer
///  @return Number of characters read into buffer
///  @detail Reads up to a cr/lf or until buffer is full
////////////////////////////////////////////////////////////////////
static int getLine()
{
  int idx = 0;
//  Serial.print("XN");
//  delay(1000);
//  Serial.print((char)XON);    // Allow chars in
//  delay(1);
//  Serial.print((char)XOFF);
//  Serial.print("XF: Avl:");
//  Serial.println(Serial.available());
  
  
  while(idx < MAX_LINE - 1)
  {
    //Serial.print("W");
    while(!Serial.available())   // wait for something 
    {
 //     Serial.print((char)XON);
      //delay(1);
 //     Serial.print((char)XOFF);
    }
    
    Serial.print("*");
    char ch = Serial.read();
    if(ch >= ' ' && ch <= '~')
    {
      lineBuffer[idx++] = toupper(ch);
      Serial.print((char)ch);
    }
    else if(ch == '\r' || ch == '\n')
    {
      lineBuffer[idx++] = '\r';
      //Serial.println();
      break;
    }
    else if(ch == '\b')
    {
       if(idx > 0)
       {
         Serial.print('\b');
         Serial.print(' ');
         Serial.print('\b');
         idx--;
       }
    }
  }
  lineBuffer[idx] = 0;
  nextCharIdx = 0;
  //Serial.println(lineBuffer);
  //Serial.print("&&&");

  //Serial.print((char)XOFF);    // Plug the faucet
  //Serial.println(">>> XOFF ----");
  return idx;
}


//////////////////////////////////////////////////////////////////
///  @fn nextChar
///  @brief Gets next character from input stream
///  @return Next character or -1 on error.
//////////////////////////////////////////////////////////////////
static int nextChar()
{
  int rtn = -1;
  while(lineBuffer[nextCharIdx] == 0)
  {
 //   Serial.print("BuffEnd");
    int count = getLine();
    if(count == 0)
    {
//      Serial.print("Cnt_0");
      break;
    }
  }
  char ch = lineBuffer[nextCharIdx++];
  if(ch != 0)
  {
    rtn = ch;
  }

  return rtn;
}


//////////////////////////////////////////////////////////
///  @fn parseCommand
///  @brief Find, parse, execute a command from the input stream.
///  @return true if a valid command is found
///  @detail Valid commands are R (read the chip), O xxxx (set 
///    offset address), C (check for blank), :xxxx (write Intel Hex
///    data), P (set Page mode), B (set Byte mode), 64 and 256 
///    (set 28C64 or 28C256 respectively), S (Motorola S record)
////////////////////////////////////////////////////////////////
static bool parseCommand()
{
  bool rtn = false;
  // skip white
  int ch;
  do
  {
    ch = nextChar();
  }while(ch <= ' ');  // skip everything SPACE and below

  switch(ch)
  {
    case 'A':     // Set Address of EEPROM
      Serial.println("Base Address setting not yet implemented.");
      {
        uint16_t val = 0;
        if(asciiToHex(4, &val))
        {
          Serial.println("Converted:");
          Serial.println(val);
        }
        else
        {
          Serial.println("Conversion failed");
        }
      }
      skipToEOL();
      break;
      
    case 'B':     // Set Byte mode
      EEPROM_setPageMode(false);
 //     Serial.println("Now using Byte mode.");
      skipToEOL();
      break;
      
    case 'C':     // Check for blank
      {
        bool isBlank = false;
        if(!EEPROM_getPower())
        {
 //         Serial.println("WARNING:  EEPROM power must be turned on ( \"O\" ) first!");
        }
        
        else
        {
           isBlank = EEPROM_isBlank();
           if(!isBlank)
           {
 //            Serial.println("WARNING:  Chip is not blank!");
           }
           else
           {
//             Serial.println("EEPROM is blank.");
           }
        }
        skipToEOL();
      }
      break;

    case 'H':     // Help -- show menu
      showMenu();
      skipToEOL();
      break;

    case 'L':     // Lock write
      Serial.println("Lock not yet implemented.");
      skipToEOL();
      break;
      
    case 'O':     // Toggle ON / OFF EEPROM power
      if(!EEPROM_getPower())
      {
        EEPROM_setPower(true);
 //       Serial.println("WARNING:  EEPROM socket now has power applied!");
      }
      else
      {
        EEPROM_setPower(false);
//        Serial.println("EEPROM power is now off.");
      }
      skipToEOL();
      break;

    case 'P':     // Set Page mode
      if(EEPROM_setPageMode(true))
      {
 //       Serial.println("Now using Page mode.");
      }
      else
      {
 //       Serial.println("WARNING: EEPROM type must be 28C256 to use Page mode.");
      }
      skipToEOL();
      break;
      
    case 'R':     // Read EEPROM, send Hex file
      if(!EEPROM_getPower())
      {
 //       Serial.println("WARNING:  EEPROM power must be turned on ( \"O\" ) first!");
      }
      else
      {
        sendIHEX();
      }
      skipToEOL();
      break;
      
    case 'S':     // Program Motorola S Record
      Serial.println("Option \"S\" (program S-Record) not yet implemented.");
      skipToEOL();
      break;

    case 'U':     // Unlock write
      Serial.println("Unlock option not yet implemented.");
      skipToEOL();
      break;

    case 'V':     // Verify hex file
      Serial.println("Verify option not yet implemented.");
      skipToEOL();
      break;

    case '6':     // 64K chip
    // TODO: check for page mode!
      if(EEPROM_setChipSize(8192))
      {
 //       Serial.println("Chip set to 28C64");
      }
      else
      {
//        Serial.println("WARNING:  Byte mode must be selected to choose 28C64");
      }
      skipToEOL();
      break;
      
    case '2':     // 256K chip
      EEPROM_setChipSize(32768);
//      Serial.println("Chip set to 28C256");
      skipToEOL();
      break;
      
    case ':':     // Intel Hex data
      parseIHEX();
      break;

    default:
      // error
      Serial.println("Invalid Command");
      showMenu();
      skipToEOL();
      break;
  }
  

  return rtn;
}


//////////////////////////////////////////////////////////////////////
///  @fn skipToEOL
///  @brief Read incoming characters up to/including CR
///  @return true for now
//////////////////////////////////////////////////////////////////////
bool skipToEOL(char lookfor)
{
  bool rtn = true;
  int ch;
  while((ch = nextChar()) != lookfor);
  return rtn;
}


//////////////////////////////////////////////////////////////////////
///  @fn parseIHEX()
///  @brief  Reads Intel Hex input, converts, and sends to EEPROM
///  @return true if valid hex input was converted and programmed
//////////////////////////////////////////////////////////////////////

static bool parseIHEX()
{
  bool rtn = true;
  uint8_t sum = 0;
  //Serial.println("Intel hex file programming not yet implemented");
  // put count into codeByteCount
  // put bytes into codeBytes[]
  // put address into codeByteAddress
  // : has already been seen when we get here
  // : count(2) addr(4) type(2) data(2*count) checksum(2)
 // Serial.println("IH");
  
  uint16_t val = 0;
  
  if(!asciiToHex(2, &val))        // byte count
  {
    //error
    rtn = false;
  }
  codeByteCount = (uint8_t)val;
  sum += (uint8_t) val;
  //Serial.print("Byte count: ");
  //Serial.println(val);
  
  if(!asciiToHex(4, &val))        // byte address
  {
    //error
    rtn = false;
  }
  codeByteAddress = val;
  sum += (uint8_t) (val & 0xff);
  sum += (uint8_t) ((val >> 4) & 0xff);
  //Serial.print("Address: ");
  //Serial.println(val);
  
  if(!asciiToHex(2, &val))        // record type
  {
    //error
    rtn = false;
  }
  // Do what with the type?
  uint8_t recordType = (uint8_t) (val);
  sum += recordType;
  //Serial.print("Record type: ");
  //Serial.println(recordType);
  
  for(int b = 0; b < codeByteCount; b++)
  {
    if(!asciiToHex(2, &val))      // Data bytes
    {
      //error
      Serial.print("Error at byte: ");
      Serial.println(b);
      rtn = false;
    }
    codeBytes[b] = (uint8_t) val;
    sum += (uint8_t) val;
    //Serial.print(b); Serial.print(" ");
    //Serial.println(val);
  }

  if(!asciiToHex(2, &val))       // Checksum
  {
    //error
    rtn = false;
  }
  sum += (uint8_t) val;          // Adding checksum should make sum == 0
  //Serial.print("Checksum: ");
  //Serial.println(val);
  if(sum != 0)
  {
    //error
    rtn = false;
  }
  if(rtn) Serial.println("Parsed IHEX line was valid.");
  
  if(!EEPROM_writeRecord(codeByteAddress, codeByteCount, codeBytes))
  {
    Serial.print("ERROR!  EEPROM verification failed at: ");
    Serial.println(codeByteAddress);
    rtn = false;
  }
  //Serial.println(rtn);


  return rtn;
}


//////////////////////////////////////////////////////////////////////
///  @fn asciiToHex
///  @brief Read Hex chars from input and return value
///  @param[in] numChars  Number of characters to convert
///  @param[out] result Pointer to place to store converted result
///  @return true if conversion was succesful
//////////////////////////////////////////////////////////////////////

static bool asciiToHex(int numChars, uint16_t* result)
{
  bool rtn = true;
  uint16_t val = 0;
  
  for(int i = 0; i < numChars; i++)
  {
    val <<= 4;
    int ch = nextChar();
    int hexval;
    if(ch >= '0' && ch <= '9')
    {
      hexval = ch - '0';
    }
    else if(ch >= 'A' && ch <= 'F')  // We converted all input to upper case
    {
      hexval = ch - 'A' + 10;
    }
    else   // Not a valid hex character
    {
      rtn = false;
      break;
    }
    val += hexval;
  }
  
  if(rtn)          // Only give them result if it is valid
  {
    *result = val;
  }

  return rtn;
}

//////////////////////////////////////////////////////////////////////////////////
///  @fn sendIHEX
///  @brief Read EEPROM and create/send Intel Hex format
///  @return 
static bool sendIHEX()
{
  bool rtn = true;
  uint8_t lineData[16];
  
  uint16_t start = EEPROM_getBaseAddress();
  uint16_t bytes = EEPROM_getChipSize();
//  interface_power(true);

  // We will read the full chip in 16 byte blocks.  If the base address 
  // is NOT some multiple of 16 bytes, the first and last records 
  // will NOT have 16 bytes.  But that would be quite unusual.

  // The first record should be just enough to get to the next multiple of record size (16)
  uint16_t nextRecordSize = ((start + OUTPUT_RECORD_SIZE - 1) - start) & ~0x000f;
  uint16_t addr = start;
  while(addr < start + bytes)
  {   
    for(int i = 0; i < nextRecordSize; i++)
    {
      lineData[i] = EEPROM_readByte(addr + i);
    }
    makeHexLine(nextRecordSize, addr, 0x0, lineData, lineBuffer);
    addr += nextRecordSize;    // Move to next record
    nextRecordSize = start + bytes - addr;
    if(nextRecordSize > OUTPUT_RECORD_SIZE)
    {
      nextRecordSize = OUTPUT_RECORD_SIZE;
    }
  }
  // Write an End Of File record
  makeHexLine(0, 0, 0x01, lineData, lineBuffer);

//  interface_power(false);
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////////
///  @fn makeHexLine
///  @brief Use passed data to create Intel Hex data record
///  @param[in] count Count of data bytes to put in record.
///  @param[in] addr  Starting address for this record
///  @param[in] rec Record type for this record.
///  @param[in] data Pointer to the data bytes.
///  @param[out] output Character array to put line into.
//////////////////////////////////////////////////////////////////////////////////
static void makeHexLine(uint8_t count, uint16_t addr, uint8_t rec, uint8_t* data, char* output)
{  
  char* nextChar = output;
  if(count <= 16)     // We didn't allocate enough space for more than 16 bytes per record
  {
    uint8_t sum = 0;
    sum = count;
    sum += (addr & 0xff);
    sum += ((addr >> 8) && 0xff);
    sum += rec;
    nextChar += sprintf(nextChar, ":%02X%04X%02X", count, addr, rec);  // put in "header"
    for(int i = 0; i < count; i++)
    {
      uint8_t b = data[i];
      sum += b;
      nextChar += sprintf(nextChar, "%02X", b);                        // Add each data byte
    }
    sum = ~sum + 1;                                                // Two's complement
    sprintf(nextChar, "%02X", sum);                                  // Add checksum
    Serial.println(output);
    
  }
}


/////////////////////////////////////////////////////////////////////
///  @fn showMenu
///  @brief  Displays a help menu of commands
/////////////////////////////////////////////////////////////////////
void showMenu()
{
  Serial.println();
  Serial.println("Commands:");
  Serial.println("A xxxx      Set base Address of EEPROM to xxxx (hex)");
  Serial.println("B           Set Byte programming mode (default)");
  Serial.println("C           Check if EEPROM is blank (add bytes 0xff");
  Serial.println("L           Lock writing of EEPROM");
  Serial.println("O           ON / OFF (toggle) EEPROM power");
  Serial.println("R           Read EEPROM to Intel Hex ");
  Serial.println("S           Write Motorola S record");
  Serial.println("U           Unlock write protection");
  Serial.println("V           Verify against hex file");
  Serial.println("2           Set EEPROM type to 28C256");
  Serial.println("6           Set EEPROM type to 28C64 (default)");
  Serial.println(":xxx...     Write Intel Hex record");
  Serial.println();
  
}
