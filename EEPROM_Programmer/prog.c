

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


FILE* infile;
FILE* outfile;

int main(int argc, char** argv)
{
  int rtn = 0;
  infile = fopen("/dev/ttyUSB0", "r+");
  outfile = fopen("/dev/ttyUSB0", "w");

  if(infile == NULL || outfile == NULL)
  {
    printf("Couldn't open a file\n");
  }
  else
  {
    fprintf(outfile, "\r\r\no\nb\n6\n");
    char input[255];
    int ch;
    while((ch = fgetc(infile)) != '\r')
    {
      if(ch < 0) printf("EOF\n");
      printf("%c", ch);
    }
    
    fgets(input, 254, infile);
    printf("RETURNED: %s", input);
  }
  




    fclose(infile);
    fclose(outfile);
    
  return rtn;
}
