;
	.zp

tmp:	.word	0
tmp2:	.word	0


	.org 0xA000
cartstart:
	sei
	cld
	; FIXME: disable NMI generator
	ldx	0xFF		; Set up stack
	txs
	; Unpack ourself into 0x0200-0x1FFF
	lda	#0xA0
	sta	@tmp+1
	lda	#0x02
	sta	@tmp2+1
	lda	#0x00
	sta	@tmp
	sta	@tmp2
	ldy	#0
	ldx	#30		; 30 pages to copy
loop:	lda	(@tmp),y
	sta	(@tmp2),y
	dey
	bne	loop
	inc	@tmp+1
	inc	@tmp2+1
	dex
	bne	loop
	jmp	0x0200		; Into the kernel discard which starts with
				; the ROM copier

;	We could run off either, but eventually we might want to
;	use OS services in the loader so run off the start method
coldstart:
	rts

	.org	0xBFFA
	.word	coldstart
	.byte	0
	.byte	0
	.word	cartstart
