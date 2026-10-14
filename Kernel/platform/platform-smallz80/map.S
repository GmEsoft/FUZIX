;
;	Mapping set up for the SmallZ80
;
;	The low 16K and high 16K are fixed, the middle 16K is switchable
;	and holds kernel or user code. The high 16K holds our common
;	(we could use either but for reboot common needs to switch in ROM
;	so it's easier to use the top)
;
;	We know the ROM mapping is already off
;
;	The _di versions of the functions are called when we know interrupts
;	are definitely off. In our case it's not useful information so both
;	symbols end up at the same code.
;

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

	.export map_buffers
	.export map_kernel
	.export map_kernel_di
	.export map_kernel_restore
	.export map_proc
	.export map_proc_a
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap

	.common

map_buffers:
	   ; for us no difference. We could potentially use a low 32K bank
	   ; for buffers but it's not clear it would gain us much value
map_kernel_restore:
map_kernel_di:
map_kernel:
	push	af
	in	a,(0xF8)
	and	0xF0		; bank 0 is kernel
	out	(0xF8),a
	pop	af
	ret
	; map_proc is called with HL either NULL or pointing to the
	; page mapping. Unlike the other calls it's allowed to trash AF
map_proc:
	ld	a, h
	or	l
	jr	z, map_kernel
map_proc_hl:
	ld	a, (hl)			; and fall through
	;
	; With a simple bank switching system you need to provide a
	; method to switch to the bank in A without corrupting any
	; other registers. The stack is safe in common memory.
	; For swap you need to provide what for simple banking is an
	; identical routine.
map_for_swap:
map_proc_a:			; used by bankfork
	push	af
	push	bc
	ld	b,a
	in	a, (0xF8)
	and	0xF0
	or	b
	out	(0xF8), a
	pop	bc
	pop	af
	ret

	;
	; Map the current process into memory. We do this by extracting
	; the bank value from u_page.
	;
map_proc_always_di:
map_proc_always:
	push af
	push hl
	ld hl, _udata + U_DATA__U_PAGE
	call map_proc_hl
	pop hl
	pop af
	ret

	;
	; Save the existing mapping and switch to the kernel.
	; The place you save it to needs to be in common memory as you
	; have no idea what bank is live. Alternatively defer the save
	; until you switch to the kernel mapping
	;
map_save_kernel:
	push	af
	in	a, (0xF8)
	and	0x0F
	ld	(mapsave), a
	in	a, (0xF8)
	and	0xF0		; kernel bank 0
	out	(0xF8), a
	pop	af
	ret
	;
	; Restore the saved bank. Note that you don't need to deal with
	; stacking of banks (we never recursively use save/restore), and
	; that we may well call save and decide not to call restore.
	;
map_restore:
	push	af
	push	hl
	ld	hl, mapsave
	in	a, (0xF8)
	or	(hl)
	out	(0xF8), a
	pop	hl
	pop	af
	ret

	.commondata

mapsave:
	.byte	0	; Saved copy of the previous map (see map_save)

