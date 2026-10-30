;
;	We keep our common area right down low, with the ZP and stack
;
;
        ; exported symbols
        .export _ub
        .export _udata
        .export kstack_top
        .export istack_top
        .export istack_switched_sp
	.export CTemp

#include "kernel.def"
#include "../../cpu-6502/kernel02.def"
;
;	In 6502 land these are the C stacks, we will need to handle the
;	hardware stack separately, and also to save sp,sp+1 etc on irqs
;
;	Declared as BSS so no non zero bytes here please
;
;	Commondata exists per process in the switched space from the REU
;
	.abs

	; Put in the 512 bytes before the process for simplicity

	.org	0x3E00

_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	.ds 512 - STASHSIZE
kstack_top:
;
;	Finally we tack the ZP save area for interrupts on the end. This
;	has to be per task as we might stash the ZP of a task in an
;	interrupt and then task switch and resume later
;
CTemp:
	.ds	STASHSIZE

; Swap space for the the C temporaries
;
	.commondata

istack_base:
	.ds	256
istack_top:
istack_switched_sp:
	.word	0
;
;	TODO set vectors up properly includign hooking kernel
;
	.common

vectors:
	.word	vector
	.word	_plt_reboot		; does it matter ???
	.word	nmi_handler

