 
////////////////////////////////////////////////////
///  CHEEPROM_202605
///  Copyright 2026 William R Cooke
////////////////////////////////////////////////////

#define HC595_DATA      (11) // 3

#define HC595_CLK       (13)
#define HC595_RCLK      (2)   //  (12)

// Data Bus:
// Low bits (D0 - D5) on PORTC (D0 - D5)
// High bits (D6 - D7) on PORTD (D6 - D7)

#define DATA_LOW            PORTC
#define DATA_HIGH           PORTD
#define DATA_LOW_IN         PINC
#define DATA_HIGH_IN        PIND
#define DATA_LOW_MASK       0x3F
#define DATA_HIGH_MASK      0xC0
#define DATA_DDR_LOW        DDRC
#define DATA_DDR_HIGH       DDRD
//#define DATA_DDR_HIGH       DDRC

// pin definitions
#define WE              (5) //14)
#define OE              (4) // 15)
#define CE              (3) // 16)
#define EEPWR0          (8)    // PB0
#define EEPWR1          (9)    // PB1

#define RED_LED         (9)
#define GRN_LED         (10)


#define SOT ( (char) 2)
#define ACK ( (char) 6)
#define NAK ( (char) 0x15)
#define EOT ( (char) 4)

// Size of output hex record
#define RECSZ   16

// Size of input line buffer
#define MAX_LINE 140

// Size of output message buffer
#define MAX_MSG 64

//////////////////////////////////////////////////
//  Global variables
//////////////////////////////////////////////////


uint8_t power = 0;                // Is the power on?
uint16_t eeprom_size = 64; // BDK8192;      // Size of EEPROM: 8192 or 32768
uint16_t base_address = 0;        // Base address of EEPROM
uint16_t current_address = 0;     // Used for "Phh" and "X" commands only


char hex_chars[] = "0123456789ABCDEF";
char str[RECSZ*2 + 16];  // "ccaaaatt {data RECSZ*2} ss \n\0"

static char input[MAX_LINE];
static char msg[MAX_MSG];

////////////////////////////////////////////////
/// @fn val
/// @brief Return value of hex digit
/// @param[in] c Character to convert, 0-9, a-f, A-F
/// @return 0 to 15 for valid hex char, -1 if invalid
//////////////////////////////////////////////////////
int val(char c)
{
  int rtn = -1;
  c = toupper(c);

  if(c >= '0' && c <= '9')
  {
    rtn = c - '0';
  }
  else if( c >= 'A' && c <= 'F')
  {
    rtn = c - 'A' + 10; 
  }
  return rtn;
}


////////////////////////////////////////
/// @fn hexval
/// @brief Gets value of hex string
/// @param[in] s String of characters
/// @param[in] cnt Number of chars to convert
/// @return Value of string, -1 if error
/////////////////////////////////////////////
long hexval(char* s, int cnt)
{
  uint16_t rtn = 0;
  for(int i = 0; i < cnt; i++)
  {
    rtn = rtn * 16 + val(s[i]);
    // BDK sprintf(msg, "%02d:%c:%d\n", i,s[i],rtn);
    // BDK send_string(msg);
  }
  return rtn;
}



/////////////////////////////////
///  @fn sync
///  @brief send sync pattern "\n\n\n"
///////////////////////////////////////
void sync()
{
  int c;
  do
  {
    Serial.write(SOT);
    delay(50);
    //Serial.flush();
    c = Serial.read();
  } while (c != ACK);
}

/////////////////////////////////////////////////
/// @fn power_on
/// @brief Turn on power to EEPROM
/////////////////////////////////////////////////
void power_on()
{
  data_bus_in();
  digitalWrite(WE, HIGH);
  PORTB |= 0x03;   // PB0 and PB1 -> Power ON
  digitalWrite(CE, HIGH);
  digitalWrite(OE, HIGH);
  power = 1;
  digitalWrite(GRN_LED, LOW);
  
}

////////////////////////////////////////////////
/// @fn power_off
/// @brief Turn off power to EEPROM
///////////////////////////////////////////////
void power_off()
{
  data_bus_in();
  PORTB &= ~0x03;  // PB0 and PB1 -> Power OFF
  digitalWrite(OE, LOW);
  digitalWrite(CE, LOW);
  digitalWrite(WE, LOW);
  write_data(0);
  write_address(0);
  power = 0;
  digitalWrite(GRN_LED, HIGH);
}

///////////////////////////////////////////////
/// @fn data_bus_in
/// @brief Set data bus to input
///////////////////////////////////////////////
void data_bus_in()
{
  DDRC &= ~DATA_LOW_MASK;
  DDRD &= ~DATA_HIGH_MASK;

}

///////////////////////////////////////////////
/// @fn data_bus_out
/// @brief Set data bus to output
///////////////////////////////////////////////
void data_bus_out()
{
  DATA_DDR_LOW |= DATA_LOW_MASK;
  DDRC |= DATA_LOW_MASK;
  DDRD |= DATA_HIGH_MASK;
}

//////////////////////////////////////////////
/// @fn read_data
/// @brief Gets data from data bus
/// @return Data read
/////////////////////////////////////////////
uint8_t read_data()
{
  uint8_t rtn = 0;
  data_bus_in();
  uint8_t t1 = DATA_LOW_IN & DATA_LOW_MASK;
  uint8_t t2 = DATA_HIGH_IN & DATA_HIGH_MASK;
  rtn = t1 | t2;

  return rtn;
}

//////////////////////////////////////////////
/// @fn read_byte
/// @brief Reads a byte from EEPROM
/// @param[in] address  The address to read from
/// @return The data read
//////////////////////////////////////////////
uint8_t read_byte(uint16_t address)
{
  uint8_t rtn = 0;
  data_bus_in();
  digitalWrite(WE, HIGH);
  digitalWrite(CE, HIGH);
  digitalWrite(OE, HIGH);
  write_address(address);
  digitalWrite(CE, LOW);
  digitalWrite(OE, LOW);
  rtn = read_data();
  digitalWrite(OE, HIGH);
  digitalWrite(CE, HIGH);
  return rtn;
}
//////////////////////////////////////////////
/// @fn write_data
/// @brief Writes data to data bus
/// @param[in] data Data to write
//////////////////////////////////////////////
void write_data(uint8_t data)
{
  uint8_t t1 = DATA_LOW_IN & ~DATA_LOW_MASK;
  t1 |= (data & DATA_LOW_MASK);
  uint8_t t2 = DATA_HIGH_IN & ~DATA_HIGH_MASK;
  t2 |= (data & DATA_HIGH_MASK);
  PORTC = t1;
  PORTD = t2;
}


///////////////////////////////////////////////
/// @fn write_address
/// @brief Output current address msb first
/// @param[in] address  The address to send
/////////////////////////////////////////////////////
void write_address(uint16_t address)
{
  for(int b = 0; b < 16; b++)
  {
    int bit = address & 0x8000;  // get high bit
    if(bit != 0)
    {
      digitalWrite(HC595_DATA, HIGH);
    }
    else
    {
      digitalWrite(HC595_DATA, LOW);
    }
    digitalWrite(HC595_CLK, HIGH);
    digitalWrite(HC595_CLK, LOW);
    address <<= 1;
  }
  // Now clock into output register
  digitalWrite(HC595_RCLK, HIGH);
  digitalWrite(HC595_RCLK, LOW);
}

////////////////////////////////////////////////////
/// @fn program_byte
/// @brief writes a data byte to EEPROM
/// @param[in] address Address to write to
/// @param[in] data Byte of data to program
/// @return 0 on success, -1 if error
////////////////////////////////////////////////////
int program_byte(uint16_t address, uint8_t data)
{
  int rtn = 0;
  write_address(address);
  write_data(data);
  data_bus_out();
  digitalWrite(CE, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  // leave CE active for verify
  
  data_bus_in();
  // wait for completion and verify
  uint8_t test = 0;
  uint16_t tries = 1000;  // somewhat arbitrary -- >= 1 ms (~1000 ish)
  do
  {
    digitalWrite(OE, LOW);
    test = read_data();
    digitalWrite(OE, HIGH);
    if(test == data)
    {
      break;
    }
  } while(--tries > 0);

  if(tries == 0)
  {
    // BDK send_string("Failed to verify: Tries = 0\n");
    rtn = -1;
  }
  digitalWrite(CE, HIGH);
  if(rtn < 0)
  {
    sprintf(msg, "Write failed: %04x:%02x ret:%02x\n", address, data, test);
    send_string(msg);
  }

  data_bus_in();
  return rtn;
}

////////////////////////////////////////////////////
/// @fn program_hex
/// @brief Programs Intel hex record into EEPROM
/// @return 0 on success, -1 if fail
////////////////////////////////////////////////////
int program_hex()
{ 
  send_string("Programming...\n");
  
  int rtn = 0;
  // hex field variables
  uint8_t cnt = 0;
  uint16_t address = 0;
  uint8_t typ = 0;
  uint8_t byt = 0;
  uint8_t cs = 0;
  
  int res = 0;
  int len = strlen(input);
  
  // :CCAAAATT [data] SS -> 11+
  //  1 3   7   9
  
  if(len < 11)
  { 
    send_string("Hex Record too short\n");
    rtn = -1;
  }
  else
  {
    cnt = (uint8_t) hexval( &input[1], 2 );
    if(len < cnt * 2 + 11)
    { 
      send_string("Hex record missing bytes\n");
      rtn = -1;
    }
    else
    {
      // check checksum
      cs = 0;
      for(int b = 0; b < (cnt + 5); b++)
      {
        uint8_t byt = hexval( &input[b * 2 + 1], 2);
        cs += byt;
      }
      
      if(cs != 0)
      {
        send_string("Hex record failed checksum\n");
        rtn = -1;
      }
      else
      {
        // BDK send_string("CS passed\n");
      }
      
      if(rtn == 0)
      {
        address = hexval( &input[3], 4 ) - base_address;
      }

      // Get the type and verify it is valid
      // 00 : Data
      // 01 : End of File (usually :00000001FF)
      // 02 : Extended segment address (:02000002aaaaSS)
      // 03 : Start Segment Address (:04000003CSIPSS)
      // 04 : Extended Linear Address (:02000004uuuuSS)
      // 05 : Start linear Address (:04000005aaaaaaaaSS)
      // other non-standard

      if(rtn == 0)  // still valid
      {
        typ = hexval( &input[7], 2 );
        if(typ != 0)  // If not a data record
        {
          send_string("Only record type 00 is supported.\n");
          rtn = -1;
        }
      }

      if(rtn == 0)
      {
        for(int i = 0; i < cnt; i++)
        {
          byt = hexval( &input[9 + i * 2], 2);
          res = program_byte(address + i, byt);
          if(res != 0) 
          {
            rtn = -1;
          }
        }
      }
    }
  }

  return rtn;
}

////////////////////////////////////////////////////
/// @fn program_data
/// @brief Programs series of bytes starting at current address
/// @return 0 on success, -1 on fail
////////////////////////////////////////////////////////////////
int program_data()
{
  int rtn = 0;
  
  int l = strlen(input) - 1; // sub off leading p
  // check we have even number of chars
  if( (l & 0x01) == 1)
  {
    rtn = -1;
  }
  else
  {
    for(int i = 0; i < l; i+=2)
    {
      long v = hexval(input + 1 + i, 2);
      if(v < 0)
      {
        sprintf(msg, "Invalid data: %c%c\n", input[i+1], input[i+2]);
        send_string(msg);
        rtn = -1;
        break;
      }
      uint8_t b = (uint8_t) v;
      int res = program_byte(current_address, b);
      if(res < 0)
      {
        sprintf(msg, "Failed to program %04X : %02X\n", current_address, b);
        send_string(msg);
        rtn = -1;
        break;
      }
      current_address++;
    }
  }
  
  return rtn;
}
////////////////////////////////////////////////////
/// @fn reset
/// @brief Reset all I/O to startup conditions
////////////////////////////////////////////////////
void reset()
{
  pinMode(EEPWR0, OUTPUT);
  pinMode(EEPWR1, OUTPUT);
  power_off();
  current_address = 0;
  pinMode(WE, OUTPUT);
  digitalWrite(WE, LOW);
  pinMode(OE, OUTPUT);
  digitalWrite(OE, LOW);
  pinMode(CE, OUTPUT);
  digitalWrite(CE, LOW);
  pinMode(HC595_DATA, OUTPUT);
  pinMode(HC595_CLK, OUTPUT);
  pinMode(HC595_RCLK, OUTPUT);
  write_address(0);
  data_bus_in();

  // BDK eeprom_size = 8192;
  current_address = 0;
  base_address = 0;
  send_string("\n\n\r\rCHEEPROM 2026 Atmel AT28Cxxx programmer\n");
  send_string("\nCopyright 2026 William R Cooke\n");
  send_string("\nVersion 1.0\n");
  send_string("Set to ...\n\r");
  send_string("AT28C64 (64K 8192 x 8) \n\r");
  send_string("base address of 0\n\r");
  
  
  
}

//////////////////////////////////////////////////
/// @fn send_byte
/// @brief Send a single byte to serial port
/// @param[in] byt The byte to send
/// @return 0 on success, -1 if error
/////////////////////////////////////////////////
int send_byte(uint8_t byt)
{
  int rtn = 0;
  Serial.write(byt);
  delay(1);

  return rtn;
}
//////////////////////////////////////////////
/// @fn send_string
/// @brief send character string to serial port
/// @param[in] s String to send (null terminated)
/// @return Number characters sent
//////////////////////////////////////////////
int send_string(char * s)
{
  int idx = 0;
  char c;
  while( (c = s[idx++]) != 0)
  {
    Serial.write(c);
    delay(1);
    //while(Serial.availableForWrite() == 0);
  }
  return idx;
}



/////////////////////////////////////////////////
/// @fn read_line
/// @brief Gets a line of input from serial port
/// @return Number of chars input
/////////////////////////////////////////////////

static int read_line()
{
  int i = 0;
  int c;
  do
  {
    c = Serial.read();
    if(c >= 0)
    {
      input[i] = (char) c;
      Serial.write( (char) c);
      delay(1);
      i++;
    }
  } while( c != '\n' && c != '\r' && i < MAX_LINE);
  input[i-1] = 0;
  
  return i;
}


/////////////////////////////////////////////////////////
/// @fn menu
/// @brief Send the menu to the PC
/////////////////////////////////////////////////////////
void menu()
{
  send_string("\n\n\n");

  send_string("M        Show this menu\n");
  send_string("U        Power Up\n");
  send_string("D        Power Down\n");
  send_string("6        Set 28C64 chip\n");
  send_string("2        Set 28C256 chip\n");
  send_string("R        Read chip contents as Intel Hex file\n");
  send_string("Axxxx    Set current address to xxxx\n");
  send_string("Bxxxx    Set base address of EEPROM to xxxx\n");
  send_string("Pxx{xx}  Progam data bytes xx starting at current address\n");
  send_string("Fxx      Fill entire chip with vale xx\n");
  
}
///////////////////////////////////////////////
/// @fn send_hex
/// @brief Sends Intel hex formatted contents of EEPROM
/////////////////////////////////////////////////////////
void send_hex()
{
  for(uint16_t rec = 0; rec < eeprom_size; rec += RECSZ)
  {

    int data = 0;
    int checksum = 0;
    char* next_ch = str;
    long next_add = rec;  // TODO add offset
    
    // Record header
    *next_ch = ':';
    next_ch++;
    
    // Count field
    checksum += 16;
    *next_ch = '1';
    next_ch++;
    *next_ch = '0';
    next_ch++;
    
    // Address field
    checksum += next_add >> 8;
    checksum += next_add & 0xff;
    for(int shift = 12; shift >= 0; shift -= 4)
    {
      *next_ch = hex_chars[(next_add >> shift) & 0x0f];
      next_ch++;
    }
    // Record type field (data record)
    // record type 0 adds 0 to checksum
    *next_ch = '0';
    next_ch++;
    *next_ch = '0';
    next_ch++;
    
    // Data field
    for(int b = 0; b < RECSZ; b++)
    {
      uint16_t rd_add = (uint16_t) (next_add + b /* BDK - base_address */ & 0xffff);
      uint8_t data = read_byte( rd_add );
      checksum += data;
      *next_ch = hex_chars[data >> 4];
      next_ch++;
      *next_ch = hex_chars[data & 0x0f];
      next_ch++;
    }
    checksum = (-checksum & 0xff);
    *next_ch = hex_chars[ (checksum >> 4) & 0x0f ];
    next_ch++;
    *next_ch = hex_chars[ checksum & 0x0f ];
    next_ch++;
    *next_ch = '\r';
    next_ch++;
    *next_ch = '\n';
    next_ch++;
    *next_ch = 0;
    send_string(str);
  }
}

//////////////////////////////////////////////
/// @fn fill_chip
/// @brief fill entire chip with single hex value
/// @return 0 on success, -1 on error
//////////////////////////////////////////////
int fill_chip()
{
  int rtn = 0;
  int l = strlen(input);
  if(l < 3) // "Fxx\n"
  {
    sprintf(msg, "cmd too short: %d\n", l);
    send_string(msg);
    rtn = -1;
  }
  else
  {
    long v = hexval(input + 1, 2);
    if(v < 0)
    {
      rtn = -1;
    }
    else
    {
      send_string("Filling...\n");
      uint8_t val = (uint8_t) v;
      for(uint16_t addr = 0; addr < eeprom_size; addr++)
      {
        int res = program_byte(addr, v);
        if(res < 0)
        {
          rtn = -1;
          sprintf(msg, "Write failed at %04x\n", addr);
        }
      }
    }
  }

  return rtn;
}
//////////////////////////////////////////////
/// @fn parse_line
/// @brief Parses and executes input line
/// @return 0 if succes, -1 if error
///
/// Commands:
/// U        Power up the EEPROM
/// D        Power down the EEPROM
/// Ahhhh    Set current address to hhhh(hex)
/// Bhhhh    Set base address of eeprom to hhhh
/// Phh      Write byte hh to current address, increment address
/// :xxx...  Program line of Intel hex data
/// R        Read entire chip as Intel hex file
/// C        Clear programmer, reset all
/// 6        Set chip to 64K
/// 2        Set chip to 256K
/// X        Examine current address byte
/// M        Print menu
/// Fhh      Fill chip with hh
///////////////////////////////////////////////

int parse_line()
{
  int rtn = 0;
  int res = 0;
  
  switch(toupper( input[0]) )
  {
    case  'U':                    // "U\n"  Power up
      power_on();
      send_string("Power up\n");
      send_byte(ACK);
      break;
      
    case 'D':                     // "D\n"  Power down
      power_off();
      send_string("Power down\n");
      send_byte(ACK);
      break;

    case 'A':                    // "Ahhhh\n" Set current address
    {
      uint8_t acknak = ACK;
      send_string(input+1);
      long address = hexval(input+1, 4);
      if(address < 0 || address > 0xffff)
      {
        send_string("\nInvalid current address value\n");
        acknak = NAK;
      }
      else
      {
        uint16_t addr = (uint16_t) address;
        current_address = addr;
        sprintf(msg, "current address set to %d\n", current_address);
        send_string(msg);
      }
      send_byte(acknak);
      break;
    }
      
    case 'B':                     // "Bhhhh\n" Set base address
    {
      uint8_t acknak = ACK;
      send_string(input+1);
      long address = hexval(input+1, 4);
      if(address < 0 || address > 0xffff)
      {
        send_string("\nInvalid base address value\n");
        acknak = NAK;
      }
      else
      {
        uint16_t addr = (uint16_t) address;
        base_address = addr;
        sprintf(msg, "\nbase address set to %d\n", base_address);
        acknak = ACK;
      }
      send_byte(acknak);
      break;
    }
    
    case 'P':                     // "Phh{hh}\n" Program consecutive bytes
      res = program_data();
      if(res == 0)
      {
        send_byte(ACK);
      }
      else
      {
        send_byte(NAK);
      }

      break;

    case ':':                     // Program Intel hex bytes
      {
        int res = program_hex();
        if(res == 0)
        {
          send_byte(ACK);
        }
        else
        {
          send_byte(NAK);
        }
        break;
      }
      
    case 'R':                     // Send Intel hex file of entire eeprom
      {
        send_hex();
        send_byte(ACK);
        break;
      }

    case '6':                     // Set 28C64
      eeprom_size = 8192;
      send_string("EEPROM set to 28C64\n");
      send_byte(ACK);
      break;

    case '2':                     // Set 28C256
      eeprom_size = 32768;
      send_string("EEPROM set to 28C256\n");
      send_byte(ACK);
      break;
      
    case 'X':                     // Examine byte at current address, inc address
      send_string("X not implemented yet\n");
      send_byte(ACK);
      break;

    case 'M':                    // Send menu
      menu();
      send_byte(ACK);
      break;

    case 'F':                    // Fill
      {
        int res = fill_chip();
        if (res == 0)
        {
          send_string("Fill success.\n");
          send_byte(ACK);
        }
        else
        {
          send_string("Fill failed.\n");
          send_byte(NAK);
        }
        break;
      }
      
    default:
      send_string("\r\n");
      send_string(input);
      char err_msg[32];
      sprintf(err_msg, "CMD: %02x\n\r", input[0]);
      send_string(err_msg);
      send_string("\r\n");
      send_string("Invalid command\r\n");
      send_byte(NAK);
      break;
  }
      

  return rtn;
}

//////////////////////////////////////////
/// @fn setup
/// @brief Set up all the things
//////////////////////////////////////////
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);


  power_off();

  reset();
  menu();
  sync();
}





////////////////////////////////////////////////////
/// @fn loop
/// @brief main program loop, called by background 
////////////////////////////////////////////////////
void loop() 
{
  
  int l = read_line();
  parse_line();


}
