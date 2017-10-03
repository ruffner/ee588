// P1P2_LAB2_EE588.c
// Code for parts one and two of Lab 2
// Fall 2017
// EE588 RTOS
//
// Modified from InputOutput example source
// InputOutput.c by Jonathan Valvano
// on March 29, 2016

#include <stdint.h>

#include "tm4c123gh6pm.h"

#define PA2 0x4
#define RED       0x02
#define BLUE      0x04
#define GREEN     0x08

void OS_Signal(int32_t *p){
  (*p)++; // not the real OS, but similar in behavior
}
void EnableInterrupts(void);

int32_t SW1,SW2 = 0;

volatile uint32_t before, elapsed;

void Switch_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x38;      // (a) activate clock for Port F and E
  SW1 = SW2 = 0;                  // (b) initialize counters
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
	GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4-0
  GPIO_PORTF_DIR_R = 0x06;        // (c) make PF4,PF0 in and PF1 and PF2 is out 
  GPIO_PORTF_DEN_R |= 0x17;       //  enable digital I/O on PF4,PF0, PF1
  GPIO_PORTF_PUR_R |= 0x11;       // pullups on PF4,PF0
  GPIO_PORTF_IS_R &= ~0x11;       // (d) PF4,PF0 are edge-sensitive 
  GPIO_PORTF_IBE_R &= ~0x11;      //     PF4,PF0 are not both edges 
  GPIO_PORTF_IEV_R &= ~0x11;      //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;        // CLEAR PORTF INT FLAGS
  GPIO_PORTF_IM_R |= 0x11;        // ARM PF.4 PF.0 INTERRUPTS	
  NVIC_EN0_R |= (1 << 30);				// ENABLE INT 30 (PF)	
	
	// PORTD
	GPIO_PORTD_LOCK_R = 0x4C4F434B; // unlock GPIO Port D
	GPIO_PORTD_CR_R = 0x08;         // allow changes to PD3
	GPIO_PORTD_DEN_R |= 0x08;  			// enable digital IO on Pd3
  GPIO_PORTD_PUR_R |= 0x08;       // pullups on PD3
	GPIO_PORTD_IS_R  &= ~0x08;      // EDGE SENSITIVE ON PD3
  GPIO_PORTD_IBE_R &= ~0x08;      // NOT BOTH EDGES 
  GPIO_PORTD_IEV_R &= ~0x08;      // FALLING EDGE
	GPIO_PORTD_ICR_R = 0x08;				// CLEAR PORT D INT FLAGS
	GPIO_PORTD_IM_R |= 0x08;				// ARM PE.3 INTERRUPT
	NVIC_EN0_R |= (1 << 3);				// ENABLE INT 3 (PD)

	// SET PRIORITIES
	NVIC_PRI7_R |= (0x5 << 21);			// PORT F PRIORITY TO 5
	NVIC_PRI4_R |= (0x4 << 29);			// PORT D PRIORITY TO 4
}

void PortF_Output(uint32_t data){   // write PF1 output
  GPIO_PORTF_DATA_R = data;      
}

void GPIOPortD_Handler(void) {
	
	PortF_Output(GPIO_PORTF_DATA_R |= RED);
	
	// ACK INTERRUPT
	GPIO_PORTD_ICR_R = 0x08;
}

void GPIOPortF_Handler(void){
	
	// record when we start handling
	before = NVIC_ST_CURRENT_R;
	
	
	
  if(GPIO_PORTF_RIS_R&0x10){  // poll PF4
    GPIO_PORTF_ICR_R = 0x10;  // acknowledge flag4
    OS_Signal(&SW1);          // signal SW1 occurred
  }
  if(GPIO_PORTF_RIS_R&0x01){  // poll PF0
    GPIO_PORTF_ICR_R = 0x01;  // acknowledge flag0
    OS_Signal(&SW2);          // signal SW2 occurred
  }
}

// from preformace example
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF;    // maximum reload value
  NVIC_ST_CURRENT_R = 0;            // any write to current clears it
  NVIC_ST_CTRL_R = 0x05;            // enable SysTick with core clock
  
}

int main(void){ 
	SysTick_Init(); // initialize SysTick timer, see SysTick.c
  Switch_Init();      // initialize PF0 and PF4 and make them inputs
  EnableInterrupts(); // I=0
  while(1){
   if(SW1){
		 elapsed = (before-NVIC_ST_CURRENT_R)&0x00FFFFFF;
     SW1=0;  // Mark it as read
     PortF_Output(GPIO_PORTF_DATA_R | 0x04); // Turn Blue LED (PF2) ON
   }
   if (SW2){
		 elapsed = (before-NVIC_ST_CURRENT_R)&0x00FFFFFF;
     SW2=0;
     PortF_Output(0x00); // Turn Red LED (PF1) OFF
   }
  }
}
