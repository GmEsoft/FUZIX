;
;	XLR8R memory map initialisation. For now we use just base memory
;	in the main machine (so 64K should work) plus the 256K on board
;
;	Internal RAM is mapped 00000-3FFFF and onboard is 40000-7FFFF.
;
;	The boot environment and OS have no understanding of the XLR8R so
;	the processor hits our boot block with the Z180 in init state with
;	wait states and all the rest of it.
;
;	The off board I/O for the keyboard is not fast enough for the XLR8R
;	in all cases so we need to manage that specially.
;
        ; exported symbols
        .export init_hardware
	.export map_io
	.export map_kernel
	.export map_kernel_di
	.export map_kernel_restore
	.export map_proc
	.export map_proc_di
	.export map_proc_a
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export _opreg
	.export _modout

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"
#include "../../cpu-z180/z180.def"

	.z180

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------

	.code

init_hardware:
	xor	a
	out0	(MMU_BBR), a	; ensure main memory then
	ld	a,0xE0		; set the high/low memory divide
	out0	(MMU_CBAR), a
	ld	hl,320		; We don't use the extended main if present TODO
        ld	(_ramsize), hl
	ld	de, 64		; for kernel
	or	a
	sbc	hl, de
        ld	(_procmem), hl

	ld	a,3
	ld	(_banktype),a	; Tell the kernel we have Z180 banking on

        ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
        ld	hl, 0
        push	hl
        call	_program_vectors
        pop	hl

        im	1 ; set CPU interrupt mode

	; interrupt mask
	; 60Hz timer on

	ld	a, 0x24		; 0x20 for serial
	out	(0xe0), a
        ret


	.commondata

opsave:
	.byte	0x06	; used as a word so keep together
extsave:
	.byte	0x01
_opreg:
	.byte	0x87	; kernel map, no I/O
_modout:
	.byte	0x50	; 80 column, sound enabled, altchars off,
			; external I/O enabled, 4MHz

	.common

map_kernel:
map_kernel_di:
map_kernel_restore:
	push	af
	ld	a, (_opreg)
	and	0x8C		; keep video bits
	or	0x03		; map 2, base memory, no I/O
	ld	(_opreg), a
	out	(0x84), a
	xor	a
	out0	(MMU_BBR),a	; main memory
	pop	af
	ret

map_io:
	push	af
	ld	a, (_opreg)
	and	0x8C		; keep video bits
	or	0x02		; map 2, base memory with I/O
	ld	(_opreg), a
	out	(0x84), a	; base memory so 0x94 doesn't matter
	xor	a
	out0	(MMU_BBR),a	; main memory
	pop	af
	ret
;
;	Userspace mapping is mode 3, U64K/L32 mapped at L64K/L32
;	Mapping codes 0x63 / 0x73. 0x94 on a bank expanded TRS80 then
;	selects how the upper bank decodes
;
map_proc:
map_proc_di:
	ld	a, h
	or	l
	jr	z, map_kernel
map_proc_hl:
	ld	a, (_opreg)
	and	0x8C
	or	0x63
	ld	(_opreg), a
	out	(0x84), a
	ld	a, (hl)	
	out0	(MMU_BBR), a
	ret

map_proc_a:			; used by bankfork
	push	af
	push	bc
	ld	b, a
	ld	a, (_opreg)
	and	0x8C
	or	0x63
	ld	(_opreg), a
	out	(0x84), a
	ld	a, b
	out0	(MMU_BBR),a
	pop	bc
	pop	af
	ret

map_proc_always:
map_proc_always_di:
	push	af
	push	hl
	ld	hl, _udata + U_DATA__U_PAGE
	call	map_proc_hl
	pop	hl
	pop	af
	ret

map_save_kernel:
	push af
	ld	a, (_opreg)
	and	0x73
	ld	(opsave), a
	in0	a,(MMU_BBR)
	ld 	(extsave), a
	ld	a, (_opreg)
	and	0x8C		; keep video bits
	or	0x03		; map 3, base memory
	ld	(_opreg), a
	out	(0x84), a
	xor	a
	out0	(MMU_BBR),a
	pop	af
	ret

map_restore:
	push	af
	push	bc
	ld	bc, (opsave)		; c = opsave b = extsave
	ld	a, (_opreg)
	and	0x8C
	or	c
	ld	(_opreg), a
	out	(0x84), a
	ld	a, b
	out0	(MMU_BBR),a
	pop	bc
	pop	af
	ret
