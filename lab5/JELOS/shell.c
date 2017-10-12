// shell.c  
//  Spring 2016
// James E. lumpp Jr.  
// 4/9/2016
//

#include <stdio.h>
#include <stdint.h>


void prmsg(char *);
int strcmp(const char *s1, const char *s2);

// -----------------------------------------------------------------
// SHELL BUILT_IN FUNCTIONS
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// FUNCTION  time:                                                  
//    Print the current time.                   
// ----------------------------------------------------------------- 

void time(void)
{  
unsigned char h,m,s;
 // Replace this with your print function or Unix time.
  printf("\nThe time is: "); 
  printf("%d:%d:%d\n",(int)h,(int)m,(int)s); // these variables will change in the background
}

// -----------------------------------------------------------------
// FUNCTION  settime:                                                  
//    Prompt the user for the current time and enter into the globals                  
// ----------------------------------------------------------------- 

void settime(char *instr)
{  
  int valid;

  do{
      printf("\n Set time to %s\n",instr);  // prompt user
 //     gets(str);  /repromt user?
		  valid=1;
  }while (valid==0);

}

void temp(void)
{  
int a;

 // Do analog to digital conversion and print the result

  printf("\nvalue is %d\n",a);  // request a single conversion
}
// -----------------------------------------------------------------
// Shell functions
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// FUNCTION  parse:                                                  
//    This function replaces all white space with zeros until it     
// reaches a character that indicates the beginning of an     
// argument.  It saves the address to argv[].                   
// ----------------------------------------------------------------- 

void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n' && *line != '\r') 
               line++;             /* skip the argument until ...    */
     }
     *argv = 0;                 /* mark the end of argument list  */
}

// -----------------------------------------------------------------                                             
//    This function will start a new process.  For now it just prints
//    The requested call                                        
// ----------------------------------------------------------------- 
     
void  execute(char **argv)
{ unsigned char i;

  printf("fork-exec: ");

  for(i=0;i<9;i++){
    if (argv[i] == 0) 
	   break;
    prmsg(argv[i]);
	  putchar(' ');
  }
  printf("\n");

}

// -----------------------------------------------------------------
// implementation of a shell
// -----------------------------------------------------------------

void  shell(void)
{
     char  line[40] = {0};          /* the input line init all chars to zero  */
     char  *argv[10] = {0};              /* the command line argument      */
//	 unsigned char i;
     
     while (1) {                   /* repeat until done ....         */
          printf("jelos# ");     /*   display a prompt             */
		      gets(line);          // get a line from the user
          parse(line, argv);       /*   parse the line               */
          if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0 ) {
		                   /* is it an "exit"?     */
                printf("Exiting...\n");
				return;

				//exit(0);            
	      } else if (strcmp(argv[0], "time") == 0)
		      time();   //time(argv[1]);
	        else if (strcmp(argv[0], "settime") == 0)
		      settime(argv[1]);   //settime(argv[1]);
          else if (strcmp(argv[0], "temp") == 0)
		      temp();   //temp(argv[1]);
					else if (strcmp(argv[0], "i") == 0)
		      puts("an i\n");   //
					else if (*argv[0] != 0 && argv[0] != 0) 
		      execute(argv);    /* if not empy line execute command as new process*/
					else	
					putchar('\n');
     }//while(1)
}

// -----------------------------------------------------------------
// main function now just launches one shell
// -----------------------------------------------------------------


         
