#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"
#include "../../cpu-z180/z180.def"

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
	.z180

map_buffers:
	   ; for us no difference. We could potentially use a low 32K bank
	   ; for buffers but it's not clear it would gain us much value
map_kernel_restore:
map_kernel_di:
map_kernel:
	push	af
	xor	a
	out0	(MMU_BBR),a		; kernel map
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
	out0	(MMU_BBR),a
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
	in0	a, (MMU_BBR)
	ld	(mapsave), a
	xor	a
	out0	(MMU_BBR),a
	pop	af
	ret
	;
	; Restore the saved bank. Note that you don't need to deal with
	; stacking of banks (we never recursively use save/restore), and
	; that we may well call save and decide not to call restore.
	;
map_restore:
	push	af
	ld	a,(mapsave)
	out0	(MMU_BBR),a
	pop	af
	ret

	.z80

	.commondata

mapsave:
	.byte	0	; Saved copy of the previous map (see map_save)

