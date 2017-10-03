;******************************************************************************
;
; timing.s - example assembly function or Keil and TMC123 dev board
; James Lumpp
; 9/10/2017

	AREA |.text|, CODE, READONLY, ALIGN=2
		
	EXPORT delay_asm


delay_asm
	subs    r0, #1
    bne     delay_asm    ; 3 clock cycle delay loop
	bx      lr

	
	ALIGN ; make sure the end of this section is aligned (For the code/data to follow)
	END ; mark end of file
