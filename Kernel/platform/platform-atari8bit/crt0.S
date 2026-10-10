#include "kernel.def"
#include "../../cpu-6502/kernel02.def"

#define	CARTPORT	0xD571
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
;	nothing else is really defined
;
go:
	sei			; interrupts off
	cld			; decimal off
	ldx	#$FF
	txs			; Stack (6502 not C)

	lda	#1
	sta	CARTPORT
	lda	#0x20
	sta	@tmp2+1		; Copy 2000-3FFF out of bank 1
	jsr	romtobank

	;	Copy from cartridge bank 2 and 3 into 4000-7FFF main RAM
	lda	#2
	jsr	copy16k

	;	Copy from cartridge bank 4 and 5 into 4000-7FFF ext bank 0
	lda	#EXT0
	sta	PORTB		; bank in at 4000-7FFF
	lda	#4		; cart bank
	jsr	copy16k

	;	Copy from cartridge bank 6 and 7 into 4000-7FFF ext bank 1
	lda	#EXT1
	sta	PORTB		; bank in at 4000-7FFF
	lda	#6		; cart bank
	jsr	copy16k

	;	Copy from cartridge bank 8 and 9 into 4000-7FFF ext bank 2
	lda	#EXT2
	sta	PORTB		; bank in at 4000-7FFF
	lda	#8		; cart bank
	jsr	copy16k

	; 	Ext bank 3 is user space so we don't need to worry
	;	We now have all the 4000-7FFF including the extra banks

	;	Back to main memory mapped
	lda	#MAIN
	sta	PORTB

	;	Now we do the upper memory (10 and 11)
	lda	#10
	sta	CARTPORT
	lda	#0xC0
	sta	@tmp2+1
	ldx	#16		; C000-CFFF (4K)
	jsr	romtosect
	lda	#0xD8		; Skip the I/O (2K)
	sta	@tmp2+1
	lda	#0xB8
	sta	@tmp+1
	ldx	#8		; Places us at E000
	jsr	romblock
	lda	#11
	sta	CARTPORT	; Final 8K from bank 11
	lda	#0xE0
	sta	@tmp2+1
	jsr	romtobank

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
	pha
	sta	CARTPORT
	jsr	romtoext
	pla
	clc
	adc	#1
	sta	CARTPORT
	bne	romtobank
;	set @tmp2 to the address to copy
romtoext:
	lda	#0x40
	sta	@tmp2+1
romtobank:
	ldx	#32
romtosect:
	lda	#>0xA0		; ROM start
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

	.common

; We may want to move some common across the top of each bank but we need
; to be careful to pack it right. Possibly also use 0x0400-0x0FFF if using
; RAMBO style expansions in future. Things to ponder TODO

	.export stub
	.export stub_end
stub:
	.ds 700
stub_end:
;
;	Overlay specific init
;
	.overlay1
	.overlay2
	.overlay3
	.overlay4		; Will use for video and buffers

screen:
	.ds	40 * 24
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

	; Then  skip 32
	.ds	32

	; We are now at 0x4400
	; Font will go here
	.ds	1024	; (or can we page before and lose 32)

	; Probably want to bank buffers here  (0x4800) if we can get
	; all of execve / user access and buffers here we can
	; do conventional rargs/wargs very very carefully.

	.export bufdata
bufdata:
	.ds	512 * NBUFS
	; We are now at about 4E00 once we allow for buffers
	; We now need all user copiers, block dev helpers, video
	; and execve/rargs/wargs stuffed into this bank

	; Hack to stop banking code turning this into a stub
	; reference
	.export _bufdata

	.abs
	.org	0x4800
_bufdata:
