#if 0
        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; 0x9000 up : code
	.area _CODE
	.area _HOME
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
	; These get overwritten and don't matter
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
	.area _PAGE0	   ; force the bin packer to leave it alone
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)

	; 0x7200 up - discard. Causes screen flicker but will get dumped
	; after boot
	.area _DISCARD
	; must end up above 0x7800
        .area _COMMONMEM
	.area _COMMONDATA
	; Stubs for the bank transfers
	.area _STUBS

	; Banked 0x4000-0x67FF
	.area _CODE1

	; Banked 0x4000-0x67FF
	.area _CODE2


	; Map over ROM as we don't really use it
        .area _INITIALIZER ; binman copies this to the right place for us

#endif

        ; exported symbols
        .export init

#include "kernelu.def"

;
;	We land here after the booter has done it's magic. The booter
;	runs in the top of the kernel bank where bss will end up
;
	.code
init:  
        di
	ld	sp, kstack_top

	;	Clear the BSS (will blow away the boot loader)
	ld	hl, __bss
	ld	de, __bss+1
	ld	bc, __bss_size-1
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
