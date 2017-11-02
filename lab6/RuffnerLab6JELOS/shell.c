// shell.c  
//  Spring 2016
// James E. lumpp Jr.  
// 4/9/2016
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "jelos.h"

extern TaskControlBlock *get_tlp(void);
extern void OS_Sem_Signal(int *s);
extern void OS_Sem_Wait(int *s);
extern int* get_sem_uart();

void prmsg(char *);
int strcmp(const char *s1, const char *s2);
extern void set_blink(uint32_t period);


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
// IMPLEMENTATION OF THE PS COMMAND
// ----------------------------------------------------------------- 
void  ps(void)
{	
	TaskControlBlock *p = get_tlp();
	uint32_t total_time = 0;
	uint32_t percent_time;
	uint32_t tms[NUM_TASKS]; // keep copy of time values in case we are switched between calculations

	#ifdef USE_SEMAPHORES
	OS_Sem_Wait(get_sem_uart());
	#endif	

	// CALCULATE TOTAL TASK TIME USAGE
	do{
		p=p->next;
		tms[p->tid] = p->ticks;
		total_time += tms[p->tid];
		//printf("\tTASK %d HAS TIME: %d\n", p->tid, tms[p->tid]);
	} while( p!=get_tlp() );
	
	//printf("TOTAL TIME: %d\n", total_time);
	
	// DISPLAY  PS INFO TO USER
	p = get_tlp();
	puts("\nUSER\tTID\t\%CPU\tSTK_SZ\t\%STK\tSTATE\tADDR\n");
	do{
		p=p->next;
		percent_time = (uint32_t)(100.0*((double)tms[p->tid]/(double)total_time));
		printf("root\t%d\t%d%%\t%d\t--\t%s\t%d\n", p->tid, percent_time, MIN_STACK_SIZE,p->state==T_CREATED ? "CREATED" : (p->state==T_READY?"READY" : "RUNNING"), (uint32_t)p->sp);
	} while( p!=get_tlp() );
	
	#ifdef USE_SEMAPHORES
	OS_Sem_Signal(get_sem_uart());
	#endif
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
	      } else if (strcmp(argv[0], "ps") == 0) 			ps();
					else if (strcmp(argv[0], "blink") == 0)   set_blink(atoi(argv[1]));
					else if (strcmp(argv[0], "time") == 0)		time();   //time(argv[1]);
	        else if (strcmp(argv[0], "settime") == 0)	settime(argv[1]);   //settime(argv[1]);
          else if (strcmp(argv[0], "temp") == 0)		temp();   //temp(argv[1]);
					else if (strcmp(argv[0], "i") == 0)				puts("an i\n");   //
					else if (*argv[0] != 0 && argv[0] != 0) 	execute(argv);    /* if not empy line execute command as new process*/
					else																			putchar('\n');
     }//while(1)
}

// -----------------------------------------------------------------
// main function now just launches one shell
// -----------------------------------------------------------------


         
