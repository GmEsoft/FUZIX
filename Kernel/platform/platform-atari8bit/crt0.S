#include "kernel.def"
#include "../../cpu-6502/kernel02.def"

#define	CARTPORT	0xD500
#define PORTB		0xD301
#define EXT0		0x12
#define EXT1		0x16
#define EXT2		0x1A
#define EXT3		0x1E
#define MAIN		0x02

;
;	We kick off in discard as we know where it is anyway.
;
	.discard
;
;	We loaded the low 8K block from cartridge and ran it. NMI is off
;	stack is sane, interrupts are off, not much else is defined.
;
go:
	lda	#0x80
	sta	0x8002

	lda	#0x00
	sta	0xD400
	lda	#<dlist8
	sta	0xD402
	lda	#>dlist8
	sta	0xD403
	lda	#0x22
	sta	0xD400

	ldx	#0xF6
	sta	CARTPORT,x	; select bank 1
	lda	#0x20
	sta	@tmp2+1		; Copy 2000-3FFF out of bank 1

	jsr	romtobank

	lda	#MAIN
	sta	PORTB

	lda	#0x80
	sta	0x8003
	;	Copy from cartridge bank 2 and 3 into 4000-7FFF main RAM
	ldx	#0xF5
	jsr	copy16k


	lda	#0x80
	sta	0x8004

	;	Copy from cartridge bank 4 and 5 into 4000-7FFF ext bank 0
	lda	#EXT0
	sta	PORTB		; bank in at 4000-7FFF


	ldx	#0xF3		; cart bank
	jsr	copy16k


	lda	#0x80
	sta	0x8005

	;	Copy from cartridge bank 6 and 7 into 4000-7FFF ext bank 1
	lda	#EXT1
	sta	PORTB		; bank in at 4000-7FFF
	ldx	#0xF1		; cart bank
	jsr	copy16k

	lda	#0x80
	sta	0x8006

	;	Copy from cartridge bank 8 and 9 into 4000-7FFF ext bank 2
	lda	#EXT2
	sta	PORTB		; bank in at 4000-7FFF
	lda	#0xE7		; cart bank
	jsr	copy16k


	lda	#0x80
	sta	0x8007

	; 	Ext bank 3 is user space so we don't need to worry
	;	We now have all the 4000-7FFF including the extra banks

	;	Back to main memory mapped
	lda	#MAIN
	sta	PORTB


	.export mark
mark:
	;	Switch to final display
	lda	#0x00
	sta	0xD400
	;	And our fonts
	lda	#>0x4F		; 0x5000 - 256
	sta	0xD409
	lda	#<dlist
	sta	0xD402
	lda	#>dlist
	sta	0xD403
	lda	#0x22
	sta	0xD400


	;	Now we do the upper memory (10 and 11)
	sta	CARTPORT+0xE5
	lda	#0xC0
	sta	@tmp2+1
	ldx	#16		; C000-CFFF (4K)
	jsr	romtosect

	lda	#0x80
	sta	0x4027


	lda	#0xD8		; Skip the I/O (2K)
	sta	@tmp2+1
	lda	#0xB8
	sta	@tmp+1
	ldx	#8		; Places us at E000
	jsr	romblock

	lda	#0x80
	sta	0x4026

	lda	CARTPORT+0xE4	; Final 8K from bank 11
	lda	#0xE0
	sta	@tmp2+1
	jsr	romtobank

	lda	#0x80
	sta	0x4025

	lda	CARTPORT+0xFF	; ROMs off IDE still accessible
	; A000-BFFF is now RAM and our memory map is set up as
	; RAM from 0000-CFFF and D800-FFFF with the I/O window
	; non removable.

	; Everything should now be in place

	lda	#<kstack_top	; C stack
	sta	@sp
	lda	#>kstack_top
	sta	@sp+1 

	lda	#<_udata
	sta	@tmp
	lda	#>_udata
	sta	@tmp+1

	lda	#0
	tay
	ldx	#2

wipeud:
	sta	(@tmp),y
	iny
	bne	wipeud
	inc	@tmp+1
	dex
	bne	wipeud

	lda	#<__bss
	sta	@tmp
	lda	#>__bss
	sta	@tmp+1

	lda	#0
	tay
	ldx	#>__bss_size
	beq	bss_wipe_tail
bss_wiper_1:
	sta	(@tmp),y
	iny
	bne	bss_wiper_1
	inc	@tmp+1
	dex
	bne	bss_wiper_1

bss_wipe_tail:
	cpy	#<__bss_size
	beq	gogogo
	sta	(@tmp),y
	iny
	bne	bss_wipe_tail

gogogo:
	jsr	init_early
	jsr	init_hardware
	jsr	_fuzix_main	; Should never return
	sei			; Spin
stop:	jmp	stop

copy16k:
	txa
	pha
	sta	CARTPORT,x
	jsr	romtoext
	pla
	tax
	dex
	sta	CARTPORT,x
	bne	romtobank
;	set @tmp2 to the address to copy
romtoext:
	lda	#0x40
	sta	@tmp2+1
romtobank:
	ldx	#32
romtosect:
	lda	#0xA0		; ROM start
	sta	@tmp+1
romblock:
	lda	#0
	sta	@tmp
	sta	@tmp2
	tay
copy:
	lda	(@tmp),y
	sta	(@tmp2),y
	iny
	bne	copy
	inc	@tmp+1
	inc	@tmp2+1
	dex
	bne	copy
	rts

vectors:
	; To copy into place TODO
	.word	vector
	.word	_plt_reboot		; does it matter ???
	.word	nmi_handler

	; scratch dlist as we will map out the cart version
	; wasteful but we have tons of discard space so who cares
dlist8:
	.byte	0x70,0x70,0x70,0x70
	.byte	0x42
	.word	0x8000
	; Render 24 lines
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	; Wait for vblank and return
	.byte	0x41
	.word	dlist8

	.common

; We may want to move some common across the top of each bank but we need
; to be careful to pack it right. Possibly also use 0x0400-0x0FFF if using
; RAMBO style expansions in future. Things to ponder TODO

	.export stub
	.export stub_end
stub:
	.ds 600
stub_end:
;
;	Overlay specific init
;
	.overlay1
	.overlay2
	.overlay3
	.overlay4

	; Overlay 4 is the main bank but the first chunk of it is
	; the buffers area (screen, disk buffers, dlist etc)
	.ds	4096

	; Font will follow at 0x5000

	; at 0x4000 in main bank - max 4K
	.buffers

	.export _bufdata
_bufdata:

	; 0x4000
	.ds	512 * NBUFS

	; 0x4A00

screen:
	.ds	40 * 25

	.ds	24

	; 0x4F00

	; Font is at 0x5000

	.commondata

	; Ensure always mapped

dlist:	.byte	0x70,0x70,0x70
	; 24 blank lines then set the start
	.byte	0x42
	.word	screen
	; Render 24 lines
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	; Wait for vblank and return
	.byte	0x41
	.word	dlist

