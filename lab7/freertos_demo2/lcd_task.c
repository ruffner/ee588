#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "switch_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "BSP.h"
#include "lcd_task.h"

#define LCDTASKSTACKSIZE        128         // Stack size in words

#define MODE_BUBBLE 1
#define MODE_TEXT   2

extern xQueueHandle g_pLCDQueue;
extern xQueueHandle buttonQueue;
extern xSemaphoreHandle g_pUARTSemaphore;

static uint8_t mode = MODE_TEXT;
static uint8_t hold = 0;
static int lastX = 0, lastY = 0;
uint16_t diffs[3]; // x,y,z

// bubble bitmap
const unsigned short bubble[] = {
 0x0000, 0x0000, 0x0000, 0x0000, 0x09A7, 0x2494, 0x2DFA, 0x2E7C, 0x2DFA, 0x2494, 0x09A7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0041, 0x2517, 0x36DE, 0x3F1F, 0x3F3F, 0x3F3F, 0x3F3F, 0x3F1F, 0x36DE, 0x24F6, 0x0041, 0x0000, 0x0000, 0x0000, 0x0041, 0x2DFA,
 0x3F1F, 0x3F1F, 0x3F1F, 0x3F1F, 0x3F1F, 0x3F1F, 0x3F1F, 0x3F1F, 0x3F1F, 0x2DFA, 0x0000, 0x0000, 0x0000, 0x1C94, 0x3F1F, 0x46FF,
 0x46FF, 0x46FF, 0x46FF, 0x46FF, 0x46FF, 0x46FF, 0x46FF, 0x46FF, 0x3F1F, 0x1C94, 0x0000, 0x0021, 0x3EDE, 0x4EFF, 0x4EBF, 0x46BE,
 0x46BE, 0x46BE, 0x46BE, 0x46BE, 0x46BE, 0x46BE, 0x46BE, 0x46FF, 0x3EDE, 0x0021, 0x1B90, 0x4EFF, 0x4E9E, 0x4E7E, 0x4E7E, 0x467E,
 0x465E, 0x465E, 0x465E, 0x465E, 0x467E, 0x4E7E, 0x4E7E, 0x4EFF, 0x1B90, 0x2D39, 0x56BF, 0x565E, 0x563E, 0x4E1E, 0x4E1E, 0x4DFE,
 0x4DFE, 0x4DFE, 0x4DFE, 0x4E1E, 0x4E1E, 0x563E, 0x56BE, 0x2D59, 0x361C, 0x4EDE, 0x4E5E, 0x465E, 0x463E, 0x463E, 0x3E1E, 0x3E1E,
 0x3E1E, 0x3E1E, 0x463E, 0x463E, 0x465E, 0x4EBE, 0x361C, 0x2D38, 0x4F1F, 0x4EDF, 0x46DE, 0x46DE, 0x3EBE, 0x3EBE, 0x3EBE, 0x3EBE,
 0x3EBE, 0x3EBE, 0x46DE, 0x46DE, 0x4F1F, 0x2D58, 0x1B70, 0x4F1F, 0x571F, 0x571F, 0x571F, 0x4F1F, 0x4F1F, 0x4F1E, 0x4F1E, 0x4F1F,
 0x4F1F, 0x571F, 0x571F, 0x4F1F, 0x1B70, 0x0021, 0x46DE, 0x675F, 0x673F, 0x673F, 0x673F, 0x673F, 0x673F, 0x673F, 0x673F, 0x673F,
 0x673F, 0x5F5F, 0x46DE, 0x0021, 0x0000, 0x2474, 0x573F, 0x775F, 0x775F, 0x775F, 0x775F, 0x775F, 0x775F, 0x775F, 0x775F, 0x775F,
 0x573F, 0x2474, 0x0000, 0x0000, 0x0041, 0x35FA, 0x5F3F, 0x7F7F, 0x877F, 0x877F, 0x877F, 0x877F, 0x877F, 0x7F7F, 0x5F3F, 0x35FA,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0041, 0x2D37, 0x4EFE, 0x673F, 0x777F, 0x7F7F, 0x777F, 0x673F, 0x4EFE, 0x2D16, 0x0041, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x09A7, 0x2CB5, 0x361B, 0x469D, 0x361B, 0x2CB5, 0x09A7, 0x0000, 0x0000, 0x0000, 0x0000,
 }; 


// helper to map different ranges
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void calcNadir(uint32_t AccX, uint32_t AccY, uint32_t AccZ, uint32_t *roll, uint32_t *pitch)
{
	double x_Buff = ((double)map(AccX,338,680,-500,500))/1000.0;
  double y_Buff = ((double)map(AccY,328,680,-500,500))/1000.0;
  double z_Buff = ((double)map(AccZ,330,730,-500,500))/1000.0;
  *pitch = atan2(y_Buff , z_Buff) * 57.3;
  *roll = atan2((- x_Buff) , sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * 57.3;
}

// draws level bubble on the display
void drawBubble(uint16_t AccX, uint16_t AccY)
{	
	int x,y;
	
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
	
}

// DISPLAY RAW XYZ ACCEL VALUES
void drawText(uint16_t AccX, uint16_t AccY, uint16_t AccZ)
{
	BSP_LCD_FillRect(lastX-7, lastY-7, 22, 22, 0x0000);
	BSP_LCD_DrawString(0, 5, "AccX=    ", BSP_LCD_Color565(255, 255, 255));
	BSP_LCD_SetCursor(5, 5);
	BSP_LCD_OutUDec((uint32_t)AccX, BSP_LCD_Color565(255, 0, 255));
	BSP_LCD_DrawString(0, 6, "AccY=    ", BSP_LCD_Color565(255, 255, 255));
	BSP_LCD_SetCursor(5, 6);
	BSP_LCD_OutUDec((uint32_t)AccY, BSP_LCD_Color565(255, 0, 255));
	BSP_LCD_DrawString(0, 7, "AccZ=    ", BSP_LCD_Color565(255, 255, 255));
	BSP_LCD_SetCursor(5, 7);
	BSP_LCD_OutUDec((uint32_t)AccZ, BSP_LCD_Color565(255, 0, 255));
	
	uint32_t Roll, Pitch;
	
	calcNadir(AccX, AccY, AccZ, &Roll, &Pitch);
	
	if(hold==2){
		uint32_t r2,p2;
		
		calcNadir(diffs[0], diffs[1], diffs[2], &r2, &p2);
		
		BSP_LCD_DrawString(10, 8, "DIFFERENCE     ", BSP_LCD_Color565(255, 255, 255));
		BSP_LCD_DrawString(0, 8, "Roll=     ", BSP_LCD_Color565(255, 255, 255));
		BSP_LCD_SetCursor(6, 8);
		BSP_LCD_OutUDec((uint32_t)abs(r2-Roll), BSP_LCD_Color565(255, 0, 255));
		BSP_LCD_DrawString(0, 9, "Pitch=    ", BSP_LCD_Color565(255, 255, 255));
		BSP_LCD_SetCursor(6, 9);
		BSP_LCD_OutUDec((uint32_t)abs(p2-Pitch), BSP_LCD_Color565(255, 0, 255));
	} else {
		BSP_LCD_DrawString(0, 8, "Roll=     ", BSP_LCD_Color565(255, 255, 255));
		BSP_LCD_SetCursor(6, 8);
		BSP_LCD_OutUDec((uint32_t)Roll, BSP_LCD_Color565(255, 0, 255));
		BSP_LCD_DrawString(0, 9, "Pitch=    ", BSP_LCD_Color565(255, 255, 255));
		BSP_LCD_SetCursor(6, 9);
		BSP_LCD_OutUDec((uint32_t)Pitch, BSP_LCD_Color565(255, 0, 255));
	}
		
}

static void
LCDTask(void *pvParameters)
{
	portTickType ui32WakeTime;
	uint32_t delay = 50;
	uint16_t accelVals[3];
	uint16_t holdVals[3];
	uint8_t buttonStatus;


	// GET CURRENT TICK COUNT
	ui32WakeTime = xTaskGetTickCount();
	
	while(1) {
		// Read the accel message
		if( xQueueReceive(g_pLCDQueue, &accelVals, 0) == pdPASS)
		{
			//xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
			//UARTprintf("Received accel values: x=%d,y=%d,z=%d\n",accelVals[0],accelVals[1],accelVals[2]);
			//xSemaphoreGive(g_pUARTSemaphore);
		}
		
		// read the next button message
		if( xQueueReceive(buttonQueue, &buttonStatus, 0) == pdPASS)
		{
			// MODE BUTTON
			if( buttonStatus & BSP_SW1_MASK ){
				// TOGGLE MODES
				mode = mode==MODE_TEXT ? MODE_BUBBLE : MODE_TEXT;
				xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
				UARTprintf("switching modes");
				xSemaphoreGive(g_pUARTSemaphore);
				
				BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
			}
			
			// LOCK BUTTON
			// 0 - no lock, show live data
			// 1 - lock, show snapshot data
			// 2 - diff, show this sample - snapshot data
			else if( buttonStatus & BSP_SW2_MASK ){
				hold = (hold+1)%3;
				if(hold==0) BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
				
				if( hold==2){
					diffs[0] = accelVals[0];
					diffs[1] = accelVals[1];
					diffs[2] = accelVals[2];
				}
				
				xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
				UARTprintf("switching hold status");
				xSemaphoreGive(g_pUARTSemaphore);
			}
		}

		// UPDATE HOLD VALUES IF WE'RE LIVE, OTHERWISE
		// PRESERVE THE LAST SAMPLE
		if( hold==0 ){
			holdVals[0] = accelVals[0];
			holdVals[1] = accelVals[1];
			holdVals[2] = accelVals[2];
		}
		
		// DECIDE WHAT TO DRAW TO SCREEN DEPENDING ON WHAT MODE
		// AND IF WE'RE HOLDING OR NOT
		if( hold ){
			if( mode == MODE_BUBBLE ){
				drawBubble(holdVals[0], holdVals[1]);
			} 
			else if( mode == MODE_TEXT ){
				drawText(holdVals[0], holdVals[1], holdVals[2]);
			}
		} else {
			if( mode == MODE_BUBBLE ){
				drawBubble(accelVals[0], accelVals[1]);
			} 
			else if( mode == MODE_TEXT ){
				drawText(accelVals[0], accelVals[1], accelVals[2]);
			}
		}
		
		
		vTaskDelayUntil(&ui32WakeTime, delay / portTICK_RATE_MS);
	}
	
}

uint32_t LCDTaskInit(void)
{
	// INITIALIZE THE LCD
	BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));

	xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
	UARTprintf("Initialized LCD\n");
	xSemaphoreGive(g_pUARTSemaphore);
	
	
	// CREATE THE LCD TASK
	if(xTaskCreate(LCDTask, (const portCHAR *)"LCD",
									 LCDTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_LCD_TASK, NULL) != pdTRUE)
	{
			return(1);
	}

	//
	// Success.
	//
	return(0);
}

