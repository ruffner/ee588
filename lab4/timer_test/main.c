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

#define PWM_PERIOD_COUNT 50000

void
TIMER2A_Handler(void)
{
	ROM_TimerIntClear(TIMER2_BASE, TIMER_CAPA_MATCH);

	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, ~ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
	
  ROM_TimerEnable(TIMER2_BASE, TIMER_A);
}

int main(void) {
	
	uint32_t pwm_limit = 0;
	uint8_t cdir = 1;
	
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
	
	
	// TIMER SETUP TO EDGE COUNT ON PF4 
	ROM_TimerConfigure (TIMER2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT));
	ROM_TimerControlEvent (TIMER2_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);
	ROM_TimerLoadSet (TIMER2_BASE, TIMER_A, 0x50);
	ROM_TimerMatchSet (TIMER2_BASE, TIMER_A, 0);
	ROM_IntEnable(INT_TIMER2A);
	ROM_TimerIntEnable(TIMER2_BASE, TIMER_CAPA_MATCH);
	ROM_TimerEnable(TIMER2_BASE, TIMER_A);



	
	// TIMER SETUP TO FADE GREEN LED PN PF3
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinConfigure(GPIO_PF3_T1CCP1);
	ROM_GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_3);
	ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
	ROM_TimerLoadSet(TIMER1_BASE, TIMER_B, PWM_PERIOD_COUNT);
	ROM_TimerMatchSet(TIMER1_BASE, TIMER_B, 0);
	ROM_TimerEnable(TIMER1_BASE, TIMER_B);


	
	
	
	// MAIN LOOP CODE ONLY CYCLES PWM LIMIT TO FADE LED
	while(1) {
		pwm_limit = cdir ? pwm_limit+100 : pwm_limit-100;
		cdir = pwm_limit <= 0 ? 1 : (pwm_limit >= PWM_PERIOD_COUNT ? 0 : cdir);
		pwm_limit = pwm_limit < 0 ? 0 : (pwm_limit >= PWM_PERIOD_COUNT ? PWM_PERIOD_COUNT : pwm_limit);
		printf("New timer Value: %d\n", pwm_limit);
		ROM_TimerMatchSet(TIMER1_BASE, TIMER_B, pwm_limit==0?1:pwm_limit );
		ROM_SysCtlDelay( pwm_limit==0?1:pwm_limit );	
	}
} 
