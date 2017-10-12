// JEL OS
// Fall 2016
// James E. Lumpp, Jr.
// 4/11/16

#include <stdlib.h>
#include <stdint.h>
#include "jelos.h"
#include <stdio.h>

static TaskControlBlock task_list[NUM_TASKS], *TASK_LIST_PTR;
static TaskControlBlock *CURRENT_TASK;

static int NEXT_TID;
static unsigned char null_task_stack[60];  // is not used, null task uses original system stack
static void InitSystem(void);
static void NullTask(void);
void EdgeCounter_Init(void);


            /* Start the multi-tasking system */
int StartScheduler(void)
	{
	if (CURRENT_TASK == NULL)
		return -1;

	EdgeCounter_Init();           // initialize GPIO Port F interrupt OR SysTick OR ...
	
  NullTask();                   // Will not return
	return 0;	 
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
	for (i = 0; i < NUM_TASKS-1; i++)
		task_list[i].next = &task_list[i+1];
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
	 CURRENT_TASK = CURRENT_TASK->next;

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
