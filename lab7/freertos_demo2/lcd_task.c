#include <stdbool.h>
#include <stdint.h>
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

static void
LCDTask(void *pvParameters)
{
	portTickType ui32WakeTime;
	uint32_t delay = 125;
	uint16_t accelVals[3];
	uint8_t buttonStatus;


	// GET CURRENT TICK COUNT
	ui32WakeTime = xTaskGetTickCount();
	
	while(1) {
		// Read the accel message
		if( xQueueReceive(g_pLCDQueue, &accelVals, 0) == pdPASS)
		{
			xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
			//UARTprintf("Received accel values: x=%d,y=%d,z=%d\n",accelVals[0],accelVals[1],accelVals[2]);
			xSemaphoreGive(g_pUARTSemaphore);
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
			}
			
			// LOCK BUTTON
			else if( buttonStatus & BSP_SW2_MASK ){
				hold = !hold;
				xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
				UARTprintf("switching hold status");
				xSemaphoreGive(g_pUARTSemaphore);
			}
		}

		vTaskDelayUntil(&ui32WakeTime, delay / portTICK_RATE_MS);
	}
	
}

uint32_t LCDTaskInit(void)
{
	// INITIALIZE THE LCD
	BSP_LCD_Init();
  //BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));

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

void drawBubble() {
	
}
