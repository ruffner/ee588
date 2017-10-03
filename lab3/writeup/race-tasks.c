static  void  Task1 (void *p_arg)
{
   (void)p_arg;
	
    while (1) {              
        BSP_LED_Toggle(1);
				UARTprintf("000000000000");	// Probably needs to be protected by semaphore
        OSTimeDlyHMSM(0, 0, 0, 1);
			}
}

static  void  Task2 (void *p_arg)
{
   (void)p_arg;
	
    while (1) {              
        BSP_LED_Toggle(2);
  			UARTprintf("111111111111");  // Probably needs to be protected by semaphore
        OSTimeDlyHMSM(0, 0, 0, 1);
			}
}