// Program to send data to serial port eeprom programmer
// Copyright 2026 William R Cooke 
//  20260511

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

// serial port
#include <fcntl.h>
#include <errno.h> 
#include <termios.h>


#define SOT 2
#define ACK 6
#define NAK 0x15
#define RESP_TIMEOUT 50

#define FILE_INPUT_BUFFER_SIZE 1024


int serial_port;
struct termios old_tty;
struct termios new_tty;

///////////////////////////////////////////////////
/// @fn setport
/// @brief Configure the serial port for CHEEPROM
/// @return 0 on success, -1 on error
///////////////////////////////////////////////////
int setport(void)
{
  int rtn = 0;
  // BDK printf("Setting port...\n");
  
  serial_port = open("/dev/ttyUSB0", O_RDWR);
  printf("Opened serial port fd=%d\n", serial_port);

  if(serial_port < 0)
  {
    printf("Error opening port: err# %d\n", errno);
    rtn = -1;
  }
  else if(tcgetattr(serial_port, &old_tty) != 0)
  {
    printf("Error getting attributes: err# %d\n", errno);
    rtn = -1;
  }
  else
  {
    // BDK printf("done...\n");
  }

// BDK   printf("Got attributes\n");
  new_tty = old_tty;
  new_tty.c_cflag &= ~PARENB;   // clear parity
  new_tty.c_cflag &= ~CSTOPB;   // One stop bit
  new_tty.c_cflag |= CS8;       // 8 bits
  new_tty.c_cflag &= ~CRTSCTS;  // diable hardware flow control
  new_tty.c_cflag += CREAD | CLOCAL;  // turn on READ and ignore ctrl lines

  // c_lflag
  new_tty.c_lflag &= ~ICANON;   // turn off canonical mode
  new_tty.c_lflag &= ~ECHO;     // turn off echo
  new_tty.c_lflag &= ~ECHOE;     // disable erasure
  new_tty.c_lflag &= ~ECHONL;    // disable newline echo

  new_tty.c_lflag &= ~ISIG;     // disable signal characters
  
  // c_iflag
  new_tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // disable special handling of bytes
  new_tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow control

  // c_oflag
  new_tty.c_oflag &= ~OPOST;  // prevent special interp of output bytes
  new_tty.c_oflag &= ~ONLCR;  // prevent conversion of newline to carriage rtn/linefeed

  // vtime / vmin
  new_tty.c_cc[VTIME] = 0;
  new_tty.c_cc[VMIN] =  0;

//  tcsetattr(serial_port,TCSAFLUSH, &new_tty);
  cfsetispeed(&new_tty, B115200);
  cfsetospeed(&new_tty, B115200);
  tcsetattr(serial_port, TCSANOW, &new_tty);
  printf("Set attributes\n");

  return rtn;
}

struct termios old_console;
struct termios new_console;
////////////////////////////////////////////////////
/// @fn set_raw_mode  depracated, remove
/// @brief puts terminal into raw mode
////////////////////////////////////////////////////
void set_raw_mode(void)
{
  // set raw mode from viewsourecode.org/snaptoken/kilo/02.enteringRawMode.html
  int res = tcgetattr(STDIN_FILENO, &old_console);
  if(res != 0)
  {
    printf("Couldn't get console attributes\n");
    return;
  }
  new_console = old_console;  // save current settings
  new_console.c_lflag &= ~(ECHO);  // turn off echo
  new_console.c_lflag &= ~(ECHO | ICANON); // echo and cananical mode
  new_console.c_lflag &= ~(ISIG);  // turn off signals ^C and ^Z
  new_console.c_lflag &= ~(IEXTEN); // turn off ^V literal key handling

  // iflags
  new_console.c_iflag &= ~(IXON);  // turn off xon/xoff
  new_console.c_iflag &= ~(ICRNL);  // turn off cr -> nl conversion
     // turn off break int, parity checking, 8 bit strip
  new_console.c_iflag &= ~(BRKINT | INPCK | ISTRIP);
  // oflags
  new_console.c_oflag &= ~(OPOST);  // turn off output conversions

  // cflags
  new_console.c_cflag |= (CS8); // char size 8 bits

  // timers
  new_console.c_cc[VMIN] = 0;  // min number of bytes to read
  new_console.c_cc[VTIME] = 1;  // tenths of second to wait for input

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_console);
}

/////////////////////////////////////////////////
/// @fn send_string
/// @brief Send a string to the programmer
/// @param[in] s  String to send
/// @return Number chars sent or  -1 on error
////////////////////////////////////////////////
int send_string(char* s)
{
  // int rtn = 0;
  int l = strlen(s);
  for(int i = 0; i < l; i++)
  {
    write(serial_port, &s[i], 1);
  }
  return l;
}

FILE* kb;       // stdin as a file stream

//////////////////////////////////////////////
/// @fn exit_fn
/// @brief Shut everything down on exit
/////////////////////////////////////////////
void exit_fn(void)
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_console);
  close(serial_port);
  fclose(kb);
}


/////////////////////////////////////////////
/// @fn get_string
/// @brief Reads a typed string from stdin
/// @param[in] s Pointer to string buffer
/// @param[in] cnt Max number of chars to read
/// @return Number of characters read
/////////////////////////////////////////////
int get_string(char* s, int cnt)
{
  int rtn = 0;
  char c;
  do
  {
    int res = read(STDIN_FILENO, &c, 1);
  }while(0);

  return rtn;
}

#define INPUT_LENGTH 600             // Will allow max length hex line (255 bytes)
static char inp[INPUT_LENGTH];       // Input line buffer
FILE* hexfile;                       // For reading an input hex file to program

//////////////////////////////////////////////////
/// @fn open_file
/// @brief Open the file in the input (inp) string.
/// @return 0 success, -1 on error
//////////////////////////////////////////////////
int open_file(char* path)
{
  int rtn = 0;
  hexfile = NULL;  // in case we've been here before
  
  int l = strlen(path);
  // skip whitespace
  char ch;
  do
  {
    path++;
    l--;
    ch = *path;
  } while(isspace(ch) && l > 0);
  if(l == 0)
  { printf("path too short\n");
    rtn = -1;
  }
  else
  {
    // find the end
    for(int i = 0; i < l; i++)
    {
      if(!isgraph(path[i]))
      {
        path[i] = '\0';
        break;
      }
    }
    printf("Opening <%s>\n", path);
    hexfile = fopen(path, "r");
    if( hexfile == NULL)
    {
      rtn = -1;
    }
  }

  return rtn;
}

//////////////////////////////////////////////////
/// @fn get_cmd
/// @brief Gets command from user and executes
/// @return -1, fail, 0 no cmd sent, 1 cmd sent, 2 quit
/// //////////////////////////////////////////////
int get_cmd(void)
{
  int rtn = 0;
  int line_length = 0;
  static int reading_hex = 0;       // Are we currently sending a hex file?
  char file_buffer[FILE_INPUT_BUFFER_SIZE];

  if(reading_hex)
  {
    char * read_result = fgets(file_buffer, FILE_INPUT_BUFFER_SIZE, hexfile);
    if(read_result == NULL)
    {
      printf("End of hex file\n");
      reading_hex = 0;
      rtn = 0;
    }
    else
    {
      int l = strlen(file_buffer);
      printf("SENDING: <%s>\n", file_buffer);
      write(serial_port, file_buffer, l);
      rtn = 1;
    }
  }
  else                             // No so get a keyboard command
  {
    printf("COMMAND:> ");
    fgets(inp, INPUT_LENGTH, kb);  // Read input from keyboard
    line_length = strlen(inp);
    if(strncasecmp(inp, "QUIT", 4) == 0 ) // quit
    {
      rtn = 2;
    }
    else if(strncasecmp(inp, "SEND", 4) == 0 ) // send file
    {
      int file_result = open_file(inp + 4);  // skip the "SEND"
      if(file_result == 0)
      {
        printf("opened file to send to programmer\n");
        reading_hex = 1;
        rtn = 0;
      }
      else
      {
        printf("Failed to open input file.\n");
      }
      rtn = 0;
    }
    else  // just send the command
    {
      write(serial_port, inp, line_length);
      rtn = 1;
    }
  }
  return rtn;
}

#define RESP_TIMEOUT 50
#define READ_SIZE    64

/////////////////////////////////////////////////////////////
/// @fn get_response
/// @brief Gets and prints response from programmer
/// @return 0: ACK, -1: NAK, -2: timeout, -3:
/////////////////////////////////////////////////////////////
int get_response(void)
{
  int rtn = 0;
  char ch = 0;
  char response[READ_SIZE];
  int timeout = RESP_TIMEOUT;
  int exit_flag = 0;

  printf("\n\nGetting...\n");

  do
  {
    int cnt = read(serial_port, response, READ_SIZE); // &ch, 1);

    if (cnt > 0)
    {
      for(int i = 0; i < cnt; i++)
      {
        ch = response[i];
        if(ch == ACK || ch == NAK )
        {
          exit_flag = 1;
          break;
        }
        putchar(response[i]);
      }
      timeout = RESP_TIMEOUT;
    }
    else
    {
      timeout--;
      if(timeout == 0)
      {
        exit_flag = 1;
      }
      sleep(1);
    }
  } while( !exit_flag);

  if( timeout == 0)
  {
    rtn = -2;
  }
  else if(ch == ACK)
  {
    rtn = 0;
  }
  else if(ch == NAK)
  {
    rtn = -1;
  }
  else
  {
    rtn = -3;
  }

  return rtn;
}

//////////////////////////////////////////////
/// @fn prog_sync
/// @brief synchronize with CHEEPROM
/// @return 1 if synced, 0 otherwise
//////////////////////////////////////////////
int prog_sync(void)
{
  int rtn = 1;
  char snc;
  do
  {
    read(serial_port, &snc, 1);
  } while( snc != SOT);

  snc = ACK;
  write(serial_port, &snc, 1);
  return 1;
}


//////////////////////////////////////////////////////////////
/// @fn main
/// @brief Get the ball rolling
/// @param[in] argc Number of arguments passed to fn
/// @param[in] argv Array of argument strings
/// @return 0 on success, error code if not
//////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  int rtn = 0;
  atexit(exit_fn);

  printf("Starting...\n");
  setport();
  printf("Set port\n");

  if(serial_port < 0)
  {
    printf("Could not open port \n");
    return -1;
  }

  char c;

  sleep(2); // 5);

  prog_sync();

  printf("\r\nSyned with CHEEPROM\r\n");

  kb = fdopen(STDIN_FILENO, "r");
  if(kb == NULL) return -1;

  atexit(exit_fn);


  int cmd_response = 0;
  int prog_response = 0;
  do
  {
    cmd_response = get_cmd();
    printf("cmd_response = %d\n", cmd_response);
    if(cmd_response == 1)  // sent a command
    {
      printf("\n\nGetting response...\n");
      prog_response = get_response();
    }
    else if(cmd_response < 0)  // error
    {
      printf("There was a command error\n");
    }
  } while (cmd_response != 2);

  // When exiting, shut off power first
  char quit_string[32];
  sprintf(quit_string, "D\n");
  write(serial_port, quit_string, strlen(quit_string));

  return rtn;
}

