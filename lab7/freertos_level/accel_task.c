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
#include "accel_task.h"

#define ACCEL_READ_DELAY        100

#define ACCELTASKSTACKSIZE        128         // Stack size in words
#define LCD_QUEUE_SIZE					5
#define LCD_ITEM_SIZE						3*sizeof(uint16_t)
	
xQueueHandle g_pLCDQueue;
extern xSemaphoreHandle g_pUARTSemaphore;

static void
AccelTask(void *pvParameters)
{
	portTickType ui32WakeTime;
	uint16_t message[3];
	
	// GET CURRENT TICK COUNT
	ui32WakeTime = xTaskGetTickCount();
	
	while(1) {
		
		// GET THE CURRENT ACCELEROMTER VALUES
		BSP_Accelerometer_Input(&message[0], &message[1], &message[2]);	
		
		//
		// Pass the value of the button pressed to LEDTask.
		//
		if(xQueueSend(g_pLCDQueue, &message, portMAX_DELAY) !=
			 pdPASS)
		{
				//
				// Error. The queue should never be full. If so print the
				// error message on UART and wait for ever.
				//
				UARTprintf("\nQueue full. This should never happen.\n");
				while(1)
				{
				}
		}
		
		vTaskDelayUntil(&ui32WakeTime, ACCEL_READ_DELAY / portTICK_RATE_MS);
	}
	
}

uint32_t AccelTaskInit(void)
{
	//
	// Init the Accelerometer
	//
	 BSP_Accelerometer_Init();


	//
	// Print the current loggling LED and frequency.
	//
	xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
	UARTprintf("Set up the Accelerometer\n");
	xSemaphoreGive(g_pUARTSemaphore);
	
	//
  // Create a queue for sending messages to the LCD task.
  //
  g_pLCDQueue = xQueueCreate(LCD_QUEUE_SIZE, LCD_ITEM_SIZE);
	
	//
	// Create the LED task.
	//
	if(xTaskCreate(AccelTask, (const portCHAR *)"Accel", ACCELTASKSTACKSIZE, NULL,
								 tskIDLE_PRIORITY + PRIORITY_ACCEL_TASK, NULL) != pdTRUE)
	{
			return(1);
	}

	//
	// Success.
	//
	return(0);
	
}
