//
//  Spring 2016 OS 
// James E. lumpp Jr.   4/6/2016
// supress warning  9/10/2017
//
  
#include <stdio.h>
#include <stdint.h> 
#include "UART.h"

void prmsg(char *p)
{
 while (*p != 0)  // Print until null terminator is reached
  putchar(*p++);
}


//extern int _textmode;

// -----------------------------------------------------------------
// implementation of C strcmp function
// -----------------------------------------------------------------

int strcmp(const char *s1, const char *s2)
{
  int ret = 0;
#pragma push
#pragma diag_suppress 1293  //supress warning:  #1293-D:  assignment in condition
  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2) 
    ++s1, ++s2;
#pragma pop
  if (ret < 0)
    ret = -1;
  else if (ret > 0)
    ret = 1 ;

  return ret;
}

// -----------------------------------------------------------------
// implementation of C gets function that accepts CR in place of newline
// -----------------------------------------------------------------

char *
 gets(char *s)
 {
         register int ch;
         register char *ptr;
         ptr = s;
         while ((ch = getchar()) != '\r')
                 *ptr++ = ch;
 
         *ptr = '\0';  // null terminate the string
         return s;
 }

 // Print a character to UART.
int fputc(int ch, FILE *f){
  if((ch == 10) || (ch == 13) || (ch == 27)){
    UART_OutChar(13);
    UART_OutChar(10);
    return 1;
  }
  UART_OutChar(ch);
  return 1;
}
// Get input from UART, echo
int fgetc (FILE *f){
  char ch = UART_InChar();  // receive from keyboard
  UART_OutChar(ch);            // echo
  return ch;
}
// Function called when file error occurs.
int ferror(FILE *f){
  /* Your implementation of ferror */
  return EOF;
}
