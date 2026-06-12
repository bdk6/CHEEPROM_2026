//////////////////////////////////////////////////////////////////////////////
///  @file EEProg.c
///  @copy 2022 William R Cooke
///  @brief Command Line Interface to EEPROM programmer
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// for serial ports
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h>
#include <termios.h>  // Contains POSIX terminal control definitions
// #include <unistd.h> // write(), read(), close()


/* Change options to match (mostly) avrdude
-p partno 2864 or 28256
-b baudrate -- don't think we need this
-B bitclock -- don't need this
-c programmer id -- don't need this
-C config file -- later
-D disable auto erase for flash -- maybe later
-e erase
-E exitspec -- don't need this
-F force -- don't need 
-i delay -- don't need
-l logfile  -- maybe later
-n no-write-disables disable writes -- don't need
-O Osc cal -- don't need
-P port 
-q quell output of status bar -- don't need
-u disable fuse read -- don't need
-s disable safemode prompt-- don't need
-t terminal mode -- don't need -- maybe later
-U general commands -- don't need
-v verbose -- maybe later
-V disable verify -- maybe later
-x extended parameter -- don't need

not in avrdude
-r read -r filename
-o offset -o 0x1234
-w write -w filename
-d direct write -d 0xadd0 0xff 0x12

final:
-C config_file
-d direct write addr data
-e erase chip
-o offset
-p partno 2864 28256
-P portname
-r read -r filename
-o offset -o 0x1234
-w write -w filename



write
open file
while not eof. && not timeout
  read line
  send.   (hex or binary?)
  wait for ack / timeout
close file

read
open file
while not end && not timeout
  send read
  get data / timeout
  save to file
  send ack
close file
*/

#define MAXLINE 523  // 1 colon 2 cnt, 4 addr, 2 type, 510 data, 2 cs, 2 eol, null
#define MAXRECEIVE 255

static char optstring[] = "C:d:eo:p:P:r:w:";
static uint16_t offset = 0;
static uint16_t chipSize = 8192;
static FILE* comport = NULL;
static FILE* configfile = NULL;
static FILE* readfile = NULL;
static FILE* writefile = NULL;
static char lineBuffer[MAXLINE];
static int fileNumber;
static char receiveData[MAXRECEIVE + 1];
static int portSet = 0;

struct termios com;
struct termios oldCom;


int sendCommand(int fnum, char* cmd);
int getResponse(int fnum, char* input, int num);
int readConfig(char* filename);
int writeDirect(char* direct);
int eraseChip(void);
int setOffset(char* offset);
int setPart(char* part);
int setPort(char* port);
int readChip(char* filename);
int writeChip(char* filename);


int main(int argc, char** argv)
{
  int op;

  
  do
  { //  "C:d:e0:p:P:r:w:";
    op = getopt(argc, argv, optstring);
    //printf("Option = %d %c \n", op, op);
    switch(op)
    {
    case 'C':  // Read config file
      //printf(" -C read config file %s \n", optarg);
      readConfig(optarg);
      break;

    case 'd':   // Write direct bytes
      //printf(" -d Write direct bytes %s \n", optarg);
      writeDirect(optarg);
      break;

    case 'e':  // Erase chip
      //printf(" -e erase chip \n");
      eraseChip();
      break;

    case 'o': // Set offset
      //printf(" -o Set offset %s \n", optarg);
      setOffset(optarg);
      break;

    case 'p':  // Set part number
      //printf(" -p Set part number %s \n", optarg);
      setPart(optarg);
      break;

    case 'P':  // Set port
      //printf(" -P Set Port %s \n", optarg);
      setPort(optarg);
      break;

    case 'r':  // Read chip to file
      //p//rintf(" -r Read chip to file %s \n", optarg);
      readChip(optarg);
      break;
      
    case 'w':  // Write file to chip
      //printf(" -w Write file %s to chip \n", optarg);
      writeChip(optarg);
      break;

    default:
      printf(" Unrecognized option %c %d \n", op, op);
      break;
    }
    
  } while(op > 0);

  //fflush(comport);
  //fprintf(comport, "0\r\n");
  //fclose(comport);
  write(fileNumber, "0\n", 2);
  tcsetattr(fileNumber, TCSAFLUSH, &oldCom);
  close(fileNumber);


  return 0;
}

int sendCommand(int fnum, char* cmd)
{
  int rtn = 0;
  int len = strlen(cmd);
  if(len > 0)
  {
    write(fnum, cmd, len);
  }
  // get response
  getResponse(fnum, receiveData, MAXRECEIVE);

  return 0;
}

int getResponse(int fnum, char* input, int num)
{
  int rtn = 0;
  read(fnum, input, num);
  printf("Len of rcv: %d \n", strlen(input));
  printf(input);
  

  return rtn;
}


int readConfig(char* filename)
{
  printf("-C configfile Not yet implemented\n");
  return -1;
}

int writeDirect(char* direct)
{
  printf("-d direct write data not yet implemented\n");
  return -1;
}

int eraseChip(void)
{
  printf("-e Erasing chip ...\n");
  sendCommand(fileNumber, "E\n");

  return 0;
}

int setOffset(char* offset)
{
  printf("-o Set offset not yet implemented \n");
  return -1;
}

int setPart(char* part)
{
  int rtn = 0;
  if(strcmp(part, "2864") == 0)
  {
    chipSize = 8192;
  }
  else if(strcmp(part, "28256") == 0)
  {
    chipSize = 32768;
  }
  else
  {
    printf("-p Part number of \"%s\" not recognized (2864 or 28256)\n", part);
    rtn = -1;
  }
  return rtn;
}

int setPort(char* port)
{
  int rtn = 0;
  printf("-P Set Port of %s \n", port);
  //comport = fopen(port, "rw");
  fileNumber = open(port, O_RDWR);
  
  printf("ran open\n");
  
  if(fileNumber < 0) //(comport == NULL)
  {
    rtn = -1;
    printf("Could not open com port %s \n", port);
  }
  else
  {
    //int num = fileno(comport);
    oldCom = com;
    portSet = 1;
    
    if(tcgetattr(fileNumber, &com) != 0)
    {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }
    else
    {
      printf("Setting\n");
      cfsetispeed(&com, B9600);
      cfsetospeed(&com, B9600);

      //disable parity
      com.c_cflag &= !PARENB;
      // one stop bit
      com.c_cflag &= !CSTOPB;
      // 8 bits
      com.c_cflag &= !CSIZE;  // clear
      com.c_cflag |= CS8;     // set 8
      // disble flow control
      com.c_cflag &= ~CRTSCTS;
      // set local mode and enable reading
      com.c_cflag |= CREAD | CREAD;

      // disable canonical mode
      com.c_lflag &= !ICANON;
      // disable echo
      com.c_lflag &= ~ECHO;
      com.c_lflag &= ~ECHOE;
      com.c_lflag &= ~ECHONL;
      // disable signal chars
      com.c_lflag &= ~ISIG;

      // disalbe sw flow control
      com.c_iflag &= ~(IXON | IXOFF | IXANY);
      // disable special handling of input bytes
      com.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

      // disable output special handling
      com.c_oflag &= ~OPOST;
      com.c_oflag &= ~ONLCR;

      com.c_cc[VMIN] = 0;
      com.c_cc[VTIME] = 100;

      
             
      if(tcsetattr(fileNumber, TCSANOW, &com) != 0)
      {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      }
      else
      {
        printf("Set port to 9600 bps\n");
      }

      char r[256];
      sleep(5);
      read(fileNumber, r, 100);
      printf(r);
      printf("Turning on \n");
      //fprintf(comport, "1\r\n");  // turn on power
      write(fileNumber, "1\r",2);
      //fflush(comport);
      printf("On\n");
      sleep(10);
      
      //fgets(r,255,comport);
      read(fileNumber, r, 100);
      printf("R<<<%s>>>\n", r);
      printf("<<<");
      for(int i = 0; i < strlen(r); i++)
      {
        printf("%d :", r[i]);
      }
      printf(">>>\n");
      sleep(15);
      printf("Off\n");
    }
  }
  
  return rtn;
}
  
  
int readChip(char* filename)
{
  int rtn = 0;
  readfile = fopen(filename, "w");
  if(readfile == NULL)
  {
    rtn = -1;
    printf("Could not open %s to save read data\n", filename);
  }
  else
  {
    // ... process
    fclose(readfile);
  }
  
  return rtn;
}

int writeChip(char* filename)
{
  int rtn = 0;
  static char lineBuff[1024];
  int lineNumber = 0;
  
  writefile = fopen(filename, "r");
  if(writefile == NULL)
  {
    rtn = -1;
    printf("Could not open %s to read programming data\n", filename);
  }
  else
  {
    // ... process
    // fgets(char* s, int size, FILE* stream);
    while(!feof(writefile))
    {
      if(fgets(lineBuffer, MAXLINE-1, writefile) == NULL)
      {
        break;
      }
      printf("Len = %d \n", strlen(lineBuffer));
      printf("LINE:%s<<", lineBuffer);
    }
    
    fclose(writefile);
  }
  
  return rtn;
}


  
