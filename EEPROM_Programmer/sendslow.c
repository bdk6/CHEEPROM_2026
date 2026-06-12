#include <stdio.h>
#include <stdint.h>
#include <string.h>

FILE* input;
FILE* output;

// place to store the data
uint8_t mem[65536];
uint8_t used[65536];  // non-zero if this byte used



typedef struct
{
  uint16_t address;
  uint8_t count;
  uint8_t rectype;
  uint8_t data[255];
} record_t;

//char buffer[520];
//record_t rec;

int hextobin(char c)
{
  int rtn = -1;
  if(c >= '0' && c <= '9')
  {
    rtn = c-'0';
  }
  else if(c >= 'a' && c <= 'f')
  {
    rtn = c - 'a' + 10;
  }
  else if(c >= 'A' && c <= 'F')
  {
    rtn = c - 'A' + 10;
  }
  return rtn;
}

int hextobin2(char c1, char c2)
{
  int rtn = 0;
  rtn += hextobin(c1) << 4;
  rtn += hextobin(c2);
  return rtn;
}


int next_char(FILE* f)
{
  int rtn = -1;
  int ch;
  do
  {
    ch = getc(f);
  } while (ch >= 0 && ch < ' ');
  rtn = ch;
  return rtn;
}

int read_byte(FILE* f)
{
  int rtn = -1;
  int ch1 = next_char(f);
  int ch2 = next_char(f);
  rtn = hextobin2(ch1, ch2);
  return rtn;
}



int read_record(FILE* f, record_t* rec)
{
  int rtn = 0;
  //int num = fgets(buff, 519, f);
  int cnt = 0;
  int address = 0;
  int typ = 0;
  int checksum = 0;

  // find ':'
  int ch;
  do
  {
    ch = next_char(f);
  } while(ch != ':' && ch >= 0);
  if(ch < 0)
  {
    // end of file
    rtn = -1;
  }
  else
  {
    int val;
    // get count
    val = read_byte(f);
    checksum += val;
    cnt = val;
    rec->count = (uint8_t) val;
    // address
    val = read_byte(f);
    checksum += val;
    address = val << 8;
    val = read_byte(f);
    checksum += val;
    address += val;
    rec->address = (uint16_t) address;
    // type
    val = read_byte(f);
    checksum += val;
    typ = val;
    rec->rectype = (uint8_t) typ;
    // data
    for(int d = 0; d < cnt; d++)
    {
      val = read_byte(f);
      rec->data[d] = (uint8_t) val;
      checksum += val;
      // TODO:  store it
    }
    // print record address and count
    printf("%04X %02X:   ", address, cnt);
    // checksum
    val = read_byte(f);
    val = (val + checksum) & 0xff;
    if(val == 0) printf("Good checksum.\n");
    else 
    {
      printf("!!!!!!!!!!!!!!!      BAD CHECKSUM       !!!!!!!!!!!!!!!!\n\n\n");
      rtn = -1;
    }

    rec->address = address;
    rec->count = cnt;
    
    return rtn;
  }
}

    


// sendslow 64 | 256 /dev/ttysXX file.hex
int main(int argc, char* argv[])
{
  int rtn = 0;

  if(argc < 4)
  {
	  printf("USAGE:  sendslow <size> <port> <file>\n");
	  printf("  where size = 64 or 256\n");
	  printf("  port is serial port (e.g. /dev/ttyUSB0)\n");
	  printf("  file is hex file (e.g. mycode.hex)\n");
	  return -1;
  }
  record_t rec;

  for(int i = 0; i < 65536; i++)
  {
	  mem[i] = 0;
	  used[i] = 0;
  }
  
  int size = 0;
  if(strcmp(argv[1], "64") == 0)
  {
	  size = 64;
  }
  else if(strcmp(argv[1], "256") == 0)
  {
	  size = 256;
  }
  else
  {
	  printf("Invalid size '%s': must be '64' or '256'\n", argv[1]);
	  return -1;
  }

  printf("Chip size: %d\n", size);
  
  output = fopen(argv[2], "w");
  // TODO 
  //if(output == NULL)
  //{
//	  printf("Could not open serial port %s for output.\n", argv[2]);
//	  return -1;
  //}
  
  input = fopen(argv[3], "r");
  if(input == NULL)
  {
	  printf("Input file %s not found\n", argv[3]);
	  return -1;
  }
  
  
  // Read in the hex file
  
  while(read_record(input, &rec) >= 0)
  {
    int a = rec.address;
    for(int d = 0; d < rec.count; d++)
    {
	    mem[a] = rec.data[d];
	    used[a] ++;
	    a++;
    }
  }

  // program it
  // open serial port
  
  for(int i = 0; i < 65536; i++)
  {
	  //if(i % 64 == 0) printf("\n");
	  if(used[i])
	  {
		  //	  printf("X");
		  printf("W %04X %02X\n", i, mem[i]);
		  // Send to serial fprintf(ser, "W%04X%02X   \n", i, mem[i]);
	  }
	  //else printf(".");
  }

  printf("\nThat's all, folks! size: %d\n\n", size);
  return rtn;
}


