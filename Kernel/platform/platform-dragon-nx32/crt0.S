	; exported
	.export start

	; Lands at start of common space for load and run

	.common
start:
	orcc	#0x10		; interrupts definitely off
	lds	#kstack_top	; note we'll wipe the stack later
	ldb	$80FD		; save from ROM space
#ifdef MOOH
	lda	#64		; enable MMU on MOOH
	sta	0xFF90
#endif
	; text and discard may be in memory bank 0
	jsr	map_kernel
	jmp	premain

	.discard

premain:
	clra
	ldx	#_udata
udata_wipe:
	sta	,x+
	cmpx	#_udata + 0x200
	blo	udata_wipe
	ldx	#__bss
	ldy	#__bss_size
bss_wipe:
	sta ,x+
	leay -1,y
	bne bss_wipe
	; This might be in BSS so don't save until we've wiped!
	clra
	cmpb	#0x49		; Dragon32
	beq	identified
	inca
	cmpb 	#0x31		; COCO1/2
	beq	identified
	inca
	cmpb	#0x32		; COCO3
	beq	identified
	inca			; Beats me
identified:
	sta	_system_id	; what sort of a box are we ?
	jsr	init_early
	jsr	init_hardware
	jmp	main

	.code

main:
	jsr _fuzix_main
	orcc #0x10		; we should never get here
stop:	bra stop
