# 1 "crt0.S"
# 45
        ; exported symbols
        .export init
# 1 "kernelu.def"
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE	.equ	0x200	; 256+256 bytes @ 0x0100



PROGBASE		.equ	0x8800
PROGLOAD		.equ	0x8800
# 50 "crt0.S"
;
;	We land here after the booter has done it's magic. The booter
;	runs in the top of the kernel bank where bss will end up
;
	.code
init:  
        di
	ld	sp, kstack_top

	;	Clear the BSS (will blow away the boot loader)
	ld	hl, __data
	ld	de, __data+1
	ld	bc, __data_size-1
	ld	(hl), 0
	ldir

	; Clear the screen
	ld	hl, 0x7000
	ld	de, 0x7001
	ld	bc, 0x01FF
	ld	(hl), 0x60	; inverted space
	ldir

        ; Hardware setup
        call	init_hardware

        ; Call the C main routine
        call	_fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr	stop

	; Where to put stubs ? TODO
	.export	stub
	.export	stub_end
stub:
	.ds	450
stub_end:

	.discard
	;	Dummy byte used to check if we have graphics or not
	.byte	0xFF
