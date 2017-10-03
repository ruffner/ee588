// PLLTestMain.c
// Runs on LM4F120/TM4C123
// Test the PLL function to verify that the system clock is
// running at the expected rate.  Use the debugger if possible
// or an oscilloscope connected to PF2.
// The #define statement in the file PLL.h allows PLL_Init() to
// initialize the PLL to the desired frequency.  When using an
// oscilloscope to look at LED1, it should be clear to see that
// the LED flashes about 2 (80/40) times faster with a 80 MHz 
// clock than with a 40 MHz clock.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 2.10, Figure 2.37

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include "PLL.h"
#include <stdint.h>
#include "tm4c123gh6pm.h"

#define BLUE 0x04

#define GPIO_PORTF2     (*((volatile uint32_t *)0x40025010))

// prototype for delay_asm in timing.s
extern void delay_asm(unsigned long t);

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
	//Code Composer Studio Code
	void Delay(unsigned long ulCount){
	__asm (	"    subs    r0, #1\n"
			"    bne     Delay\n"
			"    bx      lr\n");
}

#else
	//Keil uVision Code
	__asm void
	Delay(unsigned long ulCount)
	{
    subs    r0, #1
    bne     Delay
    bx      lr
	}

#endif
	
	
uint32_t status;

void PortF_Output(uint32_t data){   // write PF3-PF1 outputs
  GPIO_PORTF_DATA_R = data;  
}
	

int main(void){  
  PLL_Init();
  SYSCTL_RCGCGPIO_R |= 0x20;   // activate port F
  while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready?
  GPIO_PORTF_DIR_R |= 0x04;    // make PF2 out (PF2 built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04; // regular port function
  GPIO_PORTF_DEN_R |= 0x04;    // enable digital I/O on PF2
                               // configure PF4 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF0FFFF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;      // disable analog functionality on PF
  while(1){
		status = GPIO_PORTF_DATA_R;
		
		if( status & 0x04 ) {
			PortF_Output(0);
		} else {
			PortF_Output(BLUE);
		}
		
		delay_asm(1000000);
		
    //Delay(1000000);           // delay ~0.5 sec at 80 MHz
  }
}
