	.common

#include "../../build/kernelu.def"

	.export _devide_read_data
	.export _devide_write_data

idesetup:
	ld	hl,8
	add	hl,sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ex	de,hl
	ld	a, (_td_raw)
	ld	bc, 0x11		; setup port number
					; and count
	or	a
	ret	z
	cp	2
	jp	nz, map_proc_always	; User
	ld	a, (_td_page)		; Will never be zero
	jp	map_for_swap

_devide_read_data:
	push	bc
	call	idesetup
	push	af
readword:
	in	a,(0x10); Read a word
	ld	(hl),a	; Store the byte we got
	inc hl
	ini		; Now store the latched byte
	jr nz, readword
ideout:
	pop	af
	pop	bc
	or	a
	ret	z
	jp	map_kernel_restore


_devide_write_data:
	call	idesetup
	push	af
writeword:
	ld	a,(hl)
	inc	hl	; data byte
	outi		; latch a byte
	out (0x10),a	; write the data
			; and thus the latch
	jr	nz, writeword
	jr	ideout
