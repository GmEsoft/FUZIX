;
	.zp

tmp:	.word	0
tmp2:	.word	0

	.abs
	.org	0xA000

cartstart:
	sei
	cld
	sta	0xD5F7		; force a crash with wrong cart type
	lda	#0x02
	sta	0xD401
	lda	#0x00
	sta	0xD40E		; NMI off
	lda	0xD40F
;	lda	#0xE0
;	sta	0xD409		; base of char set - just so we noise
	lda	#0x00
	sta	0xD01D		; PM off
	lda	#0x0F
	sta	0xD016		; PF0
	lda	#0x0F
	sta	0xD017		; PF1
	lda	#0x70
	sta	0xD018		; PF2
;	lda	#0x70
;	sta	0xD019		; PF3
	sta	0xD01A

	;	Set up video

	sta	0xD400
	lda	#<dlist
	sta	0xD402
	lda	#>dlist	
	sta	0xD403
	lda	#0x22
	sta	0xD400
	lda	#0xD4
	ldy	#0
	lda	#0x80
	sta	0x8000

	ldx	#0xFF		; Set up stack
	txs
	; Unpack ourself into 0x0200-0x1FFF from A100-BEFF (BFxx is label)
	lda	#0xA1
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
	iny
	bne	loop
	inc	@tmp+1
	inc	@tmp2+1
	dex
	bne	loop

	lda	#0x80
	sta	0x8001

	jmp	0x0200		; Into the kernel discard which starts with
				; the ROM copier

;	We could run off either, but eventually we might want to
;	use OS services in the loader so run off the start method
coldstart:
	rts

dlist:
	.byte	0x70,0x70,0x70,0x70
	.byte	0x42
	.word	0x8000
	; Render 24 lines
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	.byte	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
	; Wait for vblank and return
	.byte	0x41
	.word	dlist

	.org	0xBFFA
	.word	coldstart
	.byte	0
	.byte	0
	.word	cartstart
