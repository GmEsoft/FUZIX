;
;	Basic VZ200 setup
;

        ; exported symbols
        .export init_hardware
	.export _program_vectors
	.export map_kernel
	.export map_proc
	.export map_proc_always
	.export map_kernel_di
	.export map_kernel_restore
	.export map_proc_di
	.export map_proc_save
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap
	.export map_buffers
	.export plt_interrupt_all
	.export _plt_reboot
	.export _plt_monitor
	.export _int_disabled
	.export _interrupt_setup
	.export _update_keyboard
	.export _sd_spi_rx_sector
	.export _sd_spi_tx_sector

	; exported debugging tools
	.export outchar

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

;=========================================================================
; Initialization code
;=========================================================================

	.discard
init_hardware:
	ld	hl, 90
	ld	(_ramsize), hl
	ld	hl, 30
	ld	(_procmem), hl
	ret

;=========================================================================
; Common Memory between 7800-87FF
;=========================================================================
	.common

_plt_monitor:
_plt_reboot:
	di
	halt

; Install the interrupt vector now the discard can be trashed
_interrupt_setup:
	ld	hl, 0x787D
	ld	(hl), 0xC3
	ld	de, interrupt_stub
	inc	 hl
	ld	(hl), e
	inc	hl
	ld	(hl), d
	ei
	xor	a
	ld	(_int_disabled), a
	inc	a
	ld	(_postinit), a
	ret

interrupt_stub:
	pop	hl		; return
	pop	hl
	pop	de
	pop	bc
	pop	af		; unwind rom
	jp	interrupt_handler	; and do our thing

	.code

; TODO: can we check the whole keyboard with a single (6800) !+ 0xFF if
; we know it was all up before ?

_update_keyboard:
	push	bc
	ld	hl, _keybuf
	ld	de, 0x68FE
	ld	b, 8	; 8 keyboard ports, 7FFE, BFFE, DFFE and so on
read_halfrow:
	ld	a, (de)
	cpl
	ld	(hl), a
	rlc	e
	inc	hl
	djnz	read_halfrow
	pop	bc
	ret

;=========================================================================

	.commondata
	.export _postinit

_int_disabled:
	.byte	1
_postinit:
	.byte	0

	.common
; install interrupt vectors
_program_vectors:
; platform fast interrupt hook
plt_interrupt_all:
	ret

spisetup:
	ld	hl, 8
	add	hl, sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ex	de,hl
	ld	bc, 57
	ld	d, 0xFF
	ld	a,(_td_raw)
	or	a
	jp	z, map_buffers
	jp	map_proc_always

_sd_spi_rx_sector:
	push	bc
	call	spisetup
readl:
	out	(c),d
	nop	; Check delay needed
	ini
	inc	b
	out	(c),d
	nop
	ini
	jr	nz, readl
	pop	bc
	jp	map_kernel_restore

_sd_spi_tx_sector:
	push	bc
	call	spisetup
writel:
	outi
	nop				; Check delay needed
	inc	b
	outi
	nop
	jr	nz, writel
	pop	bc
	jp	map_kernel_restore	

;
;	Custom interrupt handling. We have to keep interrupts off until
;	after early boot.
;
	.export _di

_di:
	jp	___hard_di

	.export _ei

_ei:
	ld	a,(_postinit)
	or	a
	ret	z
	jp	___hard_ei

	.export _irqrestore

_irqrestore:
	ld	hl, 4
	add	hl, sp
	ld	l, (hl)
	di
	ld	a,l
	ld	(_int_disabled),a
	or	a
	ret	nz
	ld	a,(_postinit)
	or	a
	ret	z
	ei
	ret

;=========================================================================
; Memory management
;=========================================================================

;
;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================

	.common

map_proc:
map_proc_di:
	ld	a,h
	or	l			; HL == 0?
	jr	z,map_kernel		; HL == 0 - map the kernel

	; fall through

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
;=========================================================================
map_for_swap:

	; fall through

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_save:		; FIXME: fix banked 
map_proc_always:
map_proc_always_di:
	push	af
	ld	a, (mapreg)
	ld	(oldmap), a	; remember our bank
	ld	a, 0x7		; RAM low, bank 1
	ld	(mapreg), a
	out	(55), a
	pop	af
	ret

;
;	On a banked kernel map_buffers will map the disk buffers and a
;	map_kernel_restore will follow. We don't need to do any work but
;	we do need to make the map_kernel_restore behave properly.
;
map_buffers:
	ld	a, (mapreg)
	ld	(oldmap), a
	ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map kernel and buffers (no difference for us)
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
map_kernel_di:
map_kernel_restore:
	push	af
	ld	a, (oldmap)
	ld	(mapreg), a
	out	(55), a
	pop	af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push	af
	ld	a, (save_map)
	ld	(mapreg), a
	out	(55), a
	pop	af
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push	af
	ld	a, (mapreg)
	ld	(save_map), a
	ld	a, 3
	ld	(mapreg), a
	out	(55), a
	pop	af
	ret

save_map:
	.byte	0
oldmap:
	.byte	0x01
mapreg:
	.byte	0x01		; start with a map setting of 1

;
;	Helpers specific to the banked singe task support
;
	.export map_save_kmap
	.export map_restore_kmap

map_save_kmap:
	ld	a, (mapreg)
	ret
map_restore_kmap:
	ld	(mapreg), a
	out	(55), a
	ret


;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
	ret

;
	.common

;	Banking support
;
	.export __bank_0_1
	.export __bank_0_2
	.export __bank_1_2
	.export __bank_2_1
	.export __stub_1
	.export __stub_2

callhl:
	jp	(hl)
__bank_0_1:
	ld	a, 1
	jr	bank0
__bank_0_2:
	ld	a, 3
bank0:
	pop	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl
	ex	de,hl
	push	bc
	ld	bc, (mapreg)	; old bank into C
	ld	(mapreg), a
	out	(55), a		; Switch bank
	bit	1, c		; Which bank ?
	pop	bc
	jr	z, retbank1
retbank2:
	call	callhl
	ld	a, 3
	ld	(mapreg), a
	out	(55), a		; and back
	ret
retbank1:
	call	callhl
	ld	a, 1
	ld	(mapreg), a
	out	(55), a
	ret
__bank_1_2:
	ld	a,3
	pop	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl
	ld	(mapreg), a
	out	(55), a
	ex	de,hl
	call	callhl
	ld	a, 1
	ld	(mapreg), a
	out	(55), a
	ret
__bank_2_1:
	ld	a, 1
	pop	hl
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	inc	hl
	push	hl
	ld	(mapreg), a
	out	(55), a
	ex	de,hl
	call	callhl
	ld	a, 3
	ld	(mapreg), a
	out	(55), a
	ret

__stub_1:
	ld	a, 1
	jr	stubin
__stub_2:
	ld	a, 3
stubin:
	pop	hl
	ex	(sp),hl
	ex	de,hl
	push	bc
	ld	bc,(mapreg)
	ld	(mapreg), a
	out	(55), a
	bit	1,c
	pop	bc
	jr	z, from_1
	call	callhl
	ld	a, 3
stubout:
	ld	(mapreg), a
	out	(55), a
	pop	af
	push	af
	push	af
	ret
from_1:
	call	callhl
	ld	a, 1
	jr	stubout
