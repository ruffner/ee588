//  Spring 2016 OS Main 
//  Author: James Lumpp
//  Date: 4/12/2016
//
// Pre-processor Directives
#include <stdio.h>  
#include <stdint.h> 
#include "UART.h"
#include "PLL.h"

// tivaware includes
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "tm4c123gh6pm.h"

void
TIMER2A_Handler(void)
{
	ROM_TimerIntClear(TIMER2_BASE, TIMER_CAPA_MATCH);

	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, ~ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
	
  ROM_TimerEnable(TIMER2_BASE, TIMER_A);
}

int main(void) {
	
	PLL_Init();     // 50 MHz (SYSDIV2 == 7, defined in pll.h)
  UART_Init();    // initialize UART
	                //115,200 baud rate (assuming 50 MHz UART clock),
                  // 8 bit word length, no parity bits, one stop bit, FIFOs enabled

  printf("\n\nWelcome to the Timer Test...\n\n");

	// PORT F GPIO INIT
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	
	ROM_SysCtlDelay(5);
	
	ROM_GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_4);
	ROM_GPIOPinConfigure(GPIO_PF4_T2CCP0);
	
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 );
	//ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	MAP_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	
	// BLINK TO SIGNAL INIT
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
	ROM_SysCtlDelay(5000000);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);	
	
	//printf("  Configured PORTF...\n");
	
	ROM_IntMasterEnable();
	
	
	ROM_TimerConfigure (TIMER2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT));
	//ROM_TimerPrescaleSet (TIMER2_BASE, TIMER_A, 0x01);
	//ROM_TimerPrescaleMatchSet (TIMER2_BASE, TIMER_A, 0x01);
	ROM_TimerControlEvent (TIMER2_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
	ROM_TimerLoadSet (TIMER2_BASE, TIMER_A, 0x50);
	ROM_TimerMatchSet (TIMER2_BASE, TIMER_A, 0);
	ROM_IntEnable(INT_TIMER2A);
	ROM_TimerIntEnable(TIMER2_BASE, TIMER_CAPA_MATCH);
	ROM_TimerEnable(TIMER2_BASE, TIMER_A);

	
	
	
	
	while(1) {
		printf("Timer Value: %d\n", ROM_TimerValueGet(TIMER2_BASE, TIMER_A));
	
		ROM_SysCtlDelay(50000);
	}
} 
