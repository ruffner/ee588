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

// tivaware includes
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

//  Global Declarations section
#ifdef USE_SEMAPHORES
unsigned int SEM_UART;												 // UART SEMAPHORE
#endif
unsigned char task_zero_stack[MIN_STACK_SIZE]; // Declare a seperate stack 
unsigned char task_one_stack[MIN_STACK_SIZE];  // for each task
unsigned char task_two_stack[MIN_STACK_SIZE];
unsigned char task_shell_stack[1024];

// Function Prototypes
void shell(void);
extern void SysTick_Init(void);
extern void OS_Sem_Signal(unsigned int *s);
extern void OS_Sem_Wait(unsigned int *s);
extern void OS_Sem_Init(unsigned int *s, unsigned int count);

void Zero(void)
	{
	while(1){ 
		#ifdef USE_SEMAPHORES 
		OS_Sem_Wait(&SEM_UART); 
		#endif
		//puts("00000000");
		#ifdef USE_SEMAPHORES 
		OS_Sem_Signal(&SEM_UART);
		#endif
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, ~ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
		
		ROM_SysCtlDelay(1000000);
		//ROM_SysCtlDelay(12500);
	}
}

void One(void)
	{

	while(1){  
		#ifdef USE_SEMAPHORES 
		OS_Sem_Wait(&SEM_UART); 
		#endif
		//puts("11111111");
		#ifdef USE_SEMAPHORES 
		OS_Sem_Signal(&SEM_UART);
		#endif
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, ~ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2));
		
		ROM_SysCtlDelay(2000000);
		//ROM_SysCtlDelay(25000);
	} 
}
	
void Two(void)
{

	while (1){
		#ifdef USE_SEMAPHORES 
		OS_Sem_Wait(&SEM_UART); 
		#endif
		//puts("22222222");
		#ifdef USE_SEMAPHORES 
		OS_Sem_Signal(&SEM_UART);
		#endif
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, ~ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3));
		
		ROM_SysCtlDelay(4000000);
		//ROM_SysCtlDelay(50000);
	}		
} 

// main
int main(void) {
	
	PLL_Init();     // 50 MHz (SYSDIV2 == 7, defined in pll.h)
  UART_Init();    // initialize UART
	                //115,200 baud rate (assuming 50 MHz UART clock),
                  // 8 bit word length, no parity bits, one stop bit, FIFOs enabled

  puts("\n\nWelcome to the Operating System...\n\n");

	// PORT F GPIO INIT
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_SysCtlDelay(5);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 );
	// BLINK TO SIGNAL INIT
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
	ROM_SysCtlDelay(5000000);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);	
	
	// Create tasks that will run (these are functions that do not return)	
	CreateTask(shell, task_shell_stack, sizeof (task_shell_stack));
	CreateTask(Zero, task_zero_stack, sizeof (task_zero_stack));
	CreateTask(One, task_one_stack, sizeof (task_one_stack));
	CreateTask(Two, task_two_stack, sizeof (task_two_stack));
		
	// INITIALIZE UART SEMAPHORE AS BINARY
	#ifdef USE_SEMAPHORES 
	OS_Sem_Init(&SEM_UART, 1);
	#endif
	
	// INIT SYSTICK FOR AUTOMATIC CONTEXT SWITCHING
	SysTick_Init();
	StartScheduler();  //Start the OS Scheduling, does not return
	
	while(1); // JIC
} //main


