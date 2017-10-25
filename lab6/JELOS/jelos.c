// JEL OS
// Fall 2016
// James E. Lumpp, Jr.
// 4/11/16

#include <stdlib.h>
#include <stdint.h>
#include "jelos.h"
#include <stdio.h>
// tivaware includes
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "tm4c123gh6pm.h"

static TaskControlBlock task_list[NUM_TASKS], *TASK_LIST_PTR;
static TaskControlBlock *CURRENT_TASK;

extern void SysTick_Handler(void);

static int NEXT_TID;
static unsigned char null_task_stack[60];  // is not used, null task uses original system stack
static void InitSystem(void);
static void NullTask(void);
void EdgeCounter_Init(void);
void SysTick_Init(void);

// FROM ASM
extern void SysTick_Handler(void);

// FOR SEMAPHORES
void OS_Suspend(void);
void OS_Sem_Signal(int *s);
void OS_Sem_Wait(int *s);
void OS_Sem_Init(int *s, int count);

            /* Start the multi-tasking system */
int StartScheduler(void)
	{
	if (CURRENT_TASK == NULL)
		return -1;

	//EdgeCounter_Init();           // initialize GPIO Port F interrupt OR SysTick OR ...
	SysTick_Init();
	
  NullTask();                   // Will not return
	return 0;	 
	}
	
// INITIALIZE SYSTICK TIMER, FOR CONTEXT SWITCHING
void SysTick_Init(void)
{
	ROM_SysTickEnable();
	ROM_SysTickPeriodSet(OS_SYSTICK_PERIOD);
	ROM_SysTickIntEnable();
	EnableInterrupts();
}
	
/* Create a new process and link it to the task list
 */
int CreateTask(void (*func)(void), 
                    unsigned char *stack_start,
                    unsigned stack_size)
					//,unsigned ticks)
	{
//	long ints;
	TaskControlBlock *p, *next;

	if (TASK_LIST_PTR == 0)
		InitSystem();
	
//	ints=StartCritical();
	p = TASK_LIST_PTR;
	TASK_LIST_PTR = TASK_LIST_PTR->next;
	p->func = func;
	p->ticks = 0;
	p->blocked = 0;
	p->state = T_CREATED;
	p->tid = NEXT_TID++;

	       /* stack grows from high address to low address */
	p->stack_start = stack_start;
	p->stack_end = stack_start+stack_size-1;
	
	p->sp = p->stack_end;

	           /* create a circular linked list */
	if (CURRENT_TASK == NULL)
		p->next = p, CURRENT_TASK = p;
	else
		next = CURRENT_TASK->next, CURRENT_TASK->next = p, p->next = next;
//  EndCritical(ints);
	return p->tid;
	}

/* Initialize the system.
 */
static void InitSystem(void)
{
	int i;

	         /* initialize the free list  */
	for (i = 0; i < NUM_TASKS-1; i++){
		task_list[i].next = &task_list[i+1];
		task_list[i].tid = 0;
	}
	TASK_LIST_PTR = &task_list[0];

	         /* null task has tid of 0 */
	CreateTask(NullTask, null_task_stack, sizeof (null_task_stack));
}


/* Always runnable task. This has the tid of zero
 */
static void NullTask(void)
	{

	while (1) 
		;          //  putchar('n');
	 
	}


// ENABLE OUTSIDE ACCESS TO TASKS
TaskControlBlock *get_tlp()
{
	return &task_list[0];
}	
	
// Schedule will save the current SP and then call teh scheduler
//	SHOULD ONLY BE CALLED IN ISR
/* Schedule(): Run a different task. Set the current task as the next one in the (circular)
 * list, then set the global variables and call the appropriate asm routines
 * to do the job. 
 */
unsigned char * Schedule(unsigned char * the_sp)  
{	
	unsigned char * sp;
  // save the current sp and schedule	
	CURRENT_TASK->sp = the_sp;
	CURRENT_TASK->state = T_READY;
	CURRENT_TASK->ticks = ROM_SysTickValueGet();
	CURRENT_TASK = CURRENT_TASK->next;
	
	while( CURRENT_TASK->blocked != 0){
		CURRENT_TASK = CURRENT_TASK->next;
	}


	if (CURRENT_TASK->state == T_READY){
		  CURRENT_TASK->state = T_RUNNING;
	    sp = CURRENT_TASK->sp;    
	} else {     /* task->state == T_CREATED so make it "ready" 
	                give it an interrupt frame and then launch it 
	    		        (with a blr sith 0xfffffff9 in LR in StartNewTask())  */
		CURRENT_TASK->state = T_RUNNING;
		sp = StartNewTask(CURRENT_TASK->sp,(uint32_t) CURRENT_TASK->func); // Does not return!
	}
	return(sp);
}

void OS_Suspend(void)
{
	NVIC_INT_CTRL_R |= 0x04000000;
}

void OS_Sem_Signal(int *s)
{
	TaskControlBlock *p = CURRENT_TASK->next;
	
	DisableInterrupts();
	*s = *s + 1;
	if( *s<=0 ){
		while( p->blocked!=s ){
			p=p->next;
		}
		p->blocked = 0;
	}
	EnableInterrupts();
}

void OS_Sem_Wait(int *s)
{
	DisableInterrupts();
	*s = *s - 1;
	if( *s<0 ){
		CURRENT_TASK->blocked = s;
		EnableInterrupts();
		OS_Suspend();
	}
	EnableInterrupts();
}

void OS_Sem_Init(int *s, int count)
{
	DisableInterrupts();
	*s = count;
	EnableInterrupts();
}
