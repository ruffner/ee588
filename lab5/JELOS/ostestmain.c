//  Spring 2016 OS Main 
//  Author: James Lumpp
//  Date: 4/12/2016
//
// Pre-processor Directives
#include <stdio.h>  
#include <stdint.h> 
#include "UART.h"
#include "PLL.h"
#include "jelos.h"

//  Global Declarations section

unsigned char task_zero_stack[MIN_STACK_SIZE]; // Declare a seperate stack 
unsigned char task_one_stack[MIN_STACK_SIZE];  // for each task
unsigned char task_two_stack[MIN_STACK_SIZE];
unsigned char task_shell_stack[1024];

// Function Prototypes
void shell(void);

void Zero(void)
	{
	while(1) 
	   putchar('0');
  //tasks should not end
	}

void One(void)
	{

	while(1)  
		putchar('1');
	
	} 
	
void Two(void)
	{

	while (1) 
		putchar('2');
  //tasks should not end
	
	} 
	
	

// main
int main(void) {
	
	PLL_Init();     // 50 MHz (SYSDIV2 == 7, defined in pll.h)
  UART_Init();    // initialize UART
	                //115,200 baud rate (assuming 50 MHz UART clock),
                  // 8 bit word length, no parity bits, one stop bit, FIFOs enabled

  puts("\n\nWelcome to the Operating System...\n\n");

	// Create tasks that will run (these are functions that do not return)
	
	CreateTask(shell, task_shell_stack, sizeof (task_shell_stack));
	CreateTask(Zero, task_zero_stack, sizeof (task_zero_stack));
	CreateTask(One, task_one_stack, sizeof (task_one_stack));
	CreateTask(Two, task_two_stack, sizeof (task_two_stack));
	
	puts("\nStarting Scheduler...");
	
	StartScheduler();  //Start the OS Scheduling, does not return
	
	while(1) // should never get here, but just in case...
		;
} //main





