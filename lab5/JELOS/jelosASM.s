;****************** 303osASM.s ***************
;
; JELOS S16
; James Lumpp
; 4/11/2016
;
; Useful declarations

GPIO_PORTF_ICR_R        EQU   0x4002541C  ; interrupt flag for PortF
	
SWITCH_COUNT EQU 0  ; Number of SYstick interrupts before a context switch
INTERRUPT_LR EQU 0xfffffff9  ; Number of SYstick interrupts before a context switch
    THUMB
		
	
	AREA DATA, ALIGN=2
		; Global variables go here

INT_COUNT SPACE 4

	EXTERN Schedule
		
	ALIGN ; make sure the end of this section is aligned
	AREA |.text|, CODE, READONLY, ALIGN=2
		
	EXPORT GPIOPortF_Handler
	EXPORT StartNewTask
		
GPIOPortF_Handler
	 ; This isr will context switch every SWITCH_COUNT ticks
	 ldr r0, =GPIO_PORTF_ICR_R
	 mov r1, #0x10
	 str r1,[r0]    ; acknowledge flag4
	 ldr r0,=INT_COUNT
	 ldr r1,[r0]
	 subs r1,r1,#1
	 blo context_sw ; perform context switch
	 str r1,[r0]
	 bx lr		    ; return from ISR
context_sw
	 mov r1,#SWITCH_COUNT
	 str r1,[r0]    ;reset INT_COUNT
	 push{r4-r11}   ; save rest of state of the task swithcing out
	 mov r0,sp
	 bl Schedule  ; will call scheduler to select new task
	 mov sp,r0          ; load new tasks sp
	 pop {r4-r11}
	 ldr lr,=INTERRUPT_LR
	 bx lr              ; context switch!

	ALIGN
		
StartNewTask
	mov sp,r0          ; stack top for this new task
	mov r2,#0x01000000  
	push {r2}           ; PSR (mark as thumb)
	push {r1}			; PC start address of the task
	ldr  r1, =task_exit   
	push {r1}     ; LR (if task ever returns)
	mov  r1,#0 ; don't care value of 0 for the other regs
	push {r1} ; r12
	push {r1} ; r3 could be arg to func
	push {r1} ; r2 could be arg to func
	push {r1} ; r1 could be arg to func
	push {r1} ; r0 could be arg to func
	ldr lr,=INTERRUPT_LR
	bx lr              ; context switch to this new tas
	
task_exit
	b	task_exit ; if a task ever returns it 
	              ; gets stuck here for debugging
	
	ALIGN ; make sure the end of this section is aligned (For the code/data to follow)
	END ; mark end of file
