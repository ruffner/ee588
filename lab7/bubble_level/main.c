// BSPTestMain.c
// Runs on any computer
// Test the general board support package by calling its
// functions to perform some operation.  To port this program
// to a different processor, re-write BSP.c for that processor.
// This file should contain nothing hardware specific and
// require no significant changes.
// Daniel and Jonathan Valvano
// February 16, 2016

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2016

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

//  J1   J3               J4   J2
// [ 1] [21]             [40] [20]
// [ 2] [22]             [39] [19]
// [ 3] [23]             [38] [18]
// [ 4] [24]             [37] [17]
// [ 5] [25]             [36] [16]
// [ 6] [26]             [35] [15]
// [ 7] [27]             [34] [14]
// [ 8] [28]             [33] [13]
// [ 9] [29]             [32] [12]
// [10] [30]             [31] [11]

// +3.3V connected to J1.1 (power)
// joystick horizontal (X) connected to J1.2 (analog)
// UART from BoosterPack to LaunchPad connected to J1.3 (UART)
// UART from LaunchPad to BoosterPack connected to J1.4 (UART)
// joystick Select button connected to J1.5 (digital)
// microphone connected to J1.6 (analog)
// LCD SPI clock connected to J1.7 (SPI)
// ambient light (OPT3001) interrupt connected to J1.8 (digital)
// ambient light (OPT3001) and temperature sensor (TMP006) I2C SCL connected to J1.9 (I2C)
// ambient light (OPT3001) and temperature sensor (TMP006) I2C SDA connected to J1.10 (I2C)

// temperature sensor (TMP006) interrupt connected to J2.11 (digital)
// nothing connected to J2.12 (SPI CS_Other)
// LCD SPI CS connected to J2.13 (SPI)
// nothing connected to J2.14 (SPI MISO)
// LCD SPI data connected to J2.15 (SPI)
// nothing connected to J2.16 (reset)
// LCD !RST connected to J2.17 (digital)
// nothing connected to J2.18 (SPI CS_Wireless)
// servo PWM connected to J2.19 (PWM)
// GND connected to J2.20 (ground)

// +5V connected to J3.21 (power)
// GND connected to J3.22 (ground)
// accelerometer X connected to J3.23 (analog)
// accelerometer Y connected to J3.24 (analog)
// accelerometer Z connected to J3.25 (analog)
// joystick vertical (Y) connected to J3.26 (analog)
// nothing connected to J3.27 (I2S WS)
// nothing connected to J3.28 (I2S SCLK)
// nothing connected to J3.29 (I2S SDout)
// nothing connected to J3.30 (I2S SDin)

// LCD RS connected to J4.31 (digital)
// user Button2 (bottom) connected to J4.32 (digital)
// user Button1 (top) connected to J4.33 (digital)
// gator hole switch connected to J4.34 (digital)
// nothing connected to J4.35
// nothing connected to J4.36
// RGB LED blue connected to J4.37 (PWM)
// RGB LED green connected to J4.38 (PWM)
// RGB LED red (jumper up) or LCD backlight (jumper down) connected to J4.39 (PWM)
// buzzer connected to J4.40 (PWM)

#include <stdint.h>
#include "bubble.h"
#include "BSP.h"
#include "tm4c123gh6pm.h"
// tivaware
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint16_t acc_hold = 0;
uint16_t AccX, AccY, AccZ;

long map(long x, long in_min, long in_max, long out_min, long out_max);


void checkbuttons(void){
	
	// negative logic
	if( BSP_Button1_Input() ){
		acc_hold = 0;
	} else {
		acc_hold = 1;
	}
	
	if( acc_hold==0 ){
		BSP_Accelerometer_Input(&AccX, &AccY, &AccZ);		
	}
}

// return the number of digits
int numlength(uint32_t n){
  if(n < 10) return 1;
  if(n < 100) return 2;
  if(n < 1000) return 3;
  if(n < 10000) return 4;
  if(n < 100000) return 5;
  if(n < 1000000) return 6;
  if(n < 10000000) return 7;
  if(n < 100000000) return 8;
  if(n < 1000000000) return 9;
  return 10;
}

int main(void){
  int count = 0;
	int x,y;
	int lastX=0, lastY=0;
	
	DisableInterrupts();
  BSP_Clock_InitFastest();
  BSP_Button1_Init();
  BSP_Button2_Init();
  
	BSP_RGB_Init(0, 0, 0);
  
	BSP_Buzzer_Init(0);
  BSP_Accelerometer_Init();
  BSP_PeriodicTask_Init(&checkbuttons, 50, 2);
  
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
	
	EnableInterrupts();
	while(1){    
		WaitForInterrupt();
    count = count + 1;
		
    if(count == 10){
      count = 0;

			BSP_LCD_FillRect(lastX-7, lastY-7, 22, 22, 0x0000);
			lastX = map(AccX,300,700,128,0);
			lastY = map(AccY,300,700,0,128);
			BSP_LCD_DrawBitmap( lastX-7, lastY+7, bubble, 15,15);  //draw bmp
			
			for( x=-11; x<=11; x++ ){
				for( y=-11; y<=11; y++ ){
					if( x*x+y*y > 100 && x*x+y*y < 110 ){
						BSP_LCD_DrawPixel(64+x, 64+y, BSP_LCD_Color565(64,255,16));
					}
				}
			}
			
      // print accelerometer status
//      BSP_LCD_DrawString(0, 5, "AccX=    ", BSP_LCD_Color565(255, 255, 255));
//      BSP_LCD_SetCursor(5, 5);
//      BSP_LCD_OutUDec((uint32_t)AccX, BSP_LCD_Color565(255, 0, 255));
//      BSP_LCD_DrawString(0, 6, "AccY=    ", BSP_LCD_Color565(255, 255, 255));
//      BSP_LCD_SetCursor(5, 6);
//      BSP_LCD_OutUDec((uint32_t)AccY, BSP_LCD_Color565(255, 0, 255));
     

    }
  }
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
