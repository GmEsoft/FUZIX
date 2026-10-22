;
;	6502 version
;
;	TODO: convert for single process model
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../../cpu-6502/kernel02.def"

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word $FE00

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_plt_switchout:
	sei

;
;	Put the C stack on the CPU stack, and store that in U_SP
;
	lda	#0		; return code
	pha
	pha
	lda	@sp		; C stack
	pha
	lda	@sp+1
	pha
	tsx
	stx	_udata + U_DATA__U_SP	; Save it

        ; find another process to run (may select this one again) returns it
        ; in x,a
        jsr	_getproc
	jmp	x1		; will be patched for banking
	nop
x1:
	jsr	__push
        jsr	_switchin
        ; we should never get here
        jsr	_plt_monitor

badswitchmsg:
	.ascii	"_switchin: FAIL"
	.byte	13, 10, 0

;
;	Any switch to another task is a swap. We should never be asked
;	to switch to the task in memory (ie current).
;
_switchin:
	sei
	jsr	__pop
	; XA holds the process to switch in
	sta	@tmp2
	stx	@tmp2+1
	; Take a second saved set as we are going to swap stacks and ZP
	; with a CPU that hasn't got sufficient registers to keep it on
	; CPU
	sta	switch_proc_ptr
	stx	switch_proc_ptr+1

;	jsr	outxa
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(@tmp2),y
	bne	not_swapped		; strange

	; Is the old task dead - in which case skip swap out
	lda	_udata + U_DATA__U_PTAB
	sta	@tmp1
	lda	_udata + U_DATA__U_PTAB+1
	sta	@tmp1+1
	lda	(@tmp1),y
	beq	old_dead

	tsx
	stx	@tmp3


	jsr	swapout		; kick the process out of memory

	; At this point the memory is free for re-use
old_dead:
	; Get our pointer back
	lda	switch_proc_ptr
	ldx	switch_proc_ptr+1
	sta	@tmp2
	stx	@tmp2+1

	ldy	#P_TAB__P_PAGE2_OFFSET
	lda	(@tmp2),y

	ldx	#0x10			; mini swap stack
	txs

	; A is the task index to swap in
	jsr	swapin

	lda	switch_proc_ptr
	ldx	switch_proc_ptr+1
	sta	@tmp2
	stx	@tmp2+1

        ; check u_data->u_ptab matches what we wanted
	lda	_udata + U_DATA__U_PTAB
	cmp	@tmp2
	bne	switchinfail
	lda	_udata + U_DATA__U_PTAB+1
	cmp	@tmp2+1
	bne	switchinfail

	lda	#P_RUNNING
	ldy	#P_TAB__P_STATUS_OFFSET
	sta	(@tmp2),y

	lda	#0
	sta	_runticks
	sta	_runticks+1

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ldx	_udata + U_DATA__U_SP
	txs

	; ------- Stack now valid again

	pla
	sta	sp+1
	pla
	sta	sp
not_swapped:
	lda	_udata + U_DATA__U_ININTERRUPT
        beq	swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pla		; Return code
	tax
	pla
        rts

switchinfail:
	lda	@tmp2+1
	jsr	outcharhex
	lda	@tmp2
	jsr	outcharhex
        lda	#<badswitchmsg
	ldx	#>badswitchmsg
        jsr	outstring
	; something went wrong and we didn't switch in what we asked for
        jmp	_plt_monitor

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
        sei	 ; should already be the case ... belt and braces.

	; new process in X, get parent pid into y
	jsr	__pop

	sta	fork_proc_ptr
	stx	fork_proc_ptr+1

	ldy	#P_TAB__P_PID_OFFSET
	sta	@tmp2
	stx	@tmp2+1

        ; Save the stack pointer and critical registers.
	; 6502 at least doesn't have too many of those 8)

        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
	lda	(@tmp2),y
	pha
	iny
	lda	(@tmp2),y
	pha

        ; save kernel stack pointer -- when it comes back in the child we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) the child PID.
	lda	@sp
	pha
	lda	@sp+1
	pha
	tsx
	stx	_udata + U_DATA__U_SP

	jsr	_tmpbuf
	jmp	x2
	nop
x2:
	sta	tmpbuf_ptr
	stx	tmpbuf_ptr+1

	; XA now points at a 512 byte work place
	sta	@tmp
	stx	@tmp+1

	lda	#<_udata
	sta	@tmp1
	lda	#>_udata
	sta	@tmp1+1
	ldy	#0
	ldx	#2
udcopy:
	lda	(@tmp),y
	sta	(@tmp1),y
	iny
	bne	udcopy
	inc	@tmp+1
	inc	@tmp1+1
	dex
	bne	udcopy

	; Ok we have the udata stashed in the tmpbuf
	; FIXME: save tmpbuf somewhere

	lda	@tmp
	ldx	@tmp+1
	jsr	__push
	lda	fork_proc_ptr
	ldx	fork_proc_ptr+1
	jsr	__push

	; Back into C to turn the tmpbuf into the child udata
	jsr	_makeproc
	jmp	x3
	nop
x3:

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	lda	fork_proc_ptr
	ldx	fork_proc_ptr+1
	sta	@tmp
	stx	@tmp+1
	ldy	#P_TAB__P_PAGE2_OFFSET
	lda	(@tmp),y

	; Y = swap slot
	jsr	swapout_new

	lda	tmpbuf_ptr
	ldx	tmpbuf_ptr+1
	jsr	__push
	jsr	_tmpfree
	jmp	x4
	nop
x4:
	; Now swap is done clean up the stack
	pla
	pla
	pla
	pla

	; Child is now on disk (yay)
	lda	fork_proc_ptr
	ldx	fork_proc_ptr+1
	sta	@tmp
	stx	@tmp+1
	ldy	#P_TAB__P_PID_OFFSET+1
	lda	(@tmp),y
	tax
	dey
	lda	(@tmp),y

	; XA is now the PID

        rts

;
;	Swappers. On most systems we do this in C via the usual disk I/O
;	but because of our 6502 ZP and stack we need to do this directly
;

swapout:
	sty	@tmp4
	jsr	slot_to_blk
	lda	#<_udata
	ldx	#>_udata
	jmp	swapit
swapout_new:
	sty	@tmp4
	jsr	slot_to_blk
	lda	tmpbuf_ptr
	ldx	tmpbuf_ptr+1
swapit:
	; Write page in XA to the disk slot
	jsr	writepage
	; Now write the ZP
	ldx	#0
	txa
	jsr	writepage
	; Now write the program out
	ldx	#0x40
writeout:
	stx	@tmp5+1
	lda	#0
	jsr	writepage
	ldx	@tmp5+1
	inx
	bne	writeout
	; Everything is now on disk
	rts

swapin:
	sta	@tmp4
	jsr	slot_to_blk
	lda	#<_udata
	ldx	#>_udata
	jsr	readpage
	lda	#0
	tax
	jsr	readpage
	ldx	#0x40
readin:
	stx	@tmp5+1
	lda	#0
	jsr	readpage
	ldx	@tmp5+1
	inx
	bne	readin
	; Everything is now back in RAM
	rts

disksetup:
	;	Transfer from user as swapping (udata and ZP/S are always
	; 	mapped anyway)
	lda	#1
	sta	_td_raw
	lda	_swapdevh
	sta	0xDE26
	; TODO wait for ready
	lda	@tmp5
	sta	0xDE25
	lda	@tmp4+1
	sta	0xDE24
	lda	@tmp4
	sta	0xDE23
	lda	#1
	sta	0xDE22
	; TODO wait for ready
	sty	0xDE27		; command
	; TODO wait for DRQ
	rts

writepage:
	; Write 512 bytes from the main memory map at XA to disk
	; @tmp4/5low counts the disk block number relative to the swap
	; partition
	stx	@tmp+1
	sta	@tmp
	ldy	#0x20		; READ
	jsr	disksetup
	ldx	@tmp+1
	lda	@tmp
	jsr	devide_write_data
	; TODO wait for DRQ drop/ready
	jmp	diskstat

readpage:
	; Read 512 bytes from disk into the main memory map at XA
	; magically avoids reading over the kernel @tmp and lowest 16 bytes
	; of page 1
	; This however forces us to do all the work in asm unlike "normal"
	; ports
	; @tmp4/5low counts the disk block numnber
	stx	@tmp+1
	sta	@tmp
	ldy	#0x30		; WRITE
	jsr	disksetup
	; Do the disk command, now transfer the bytes
	ldx	@tmp+1
	lda	@tmp
	jsr	devide_read_data
diskstat:
	lda	0xD527
	pha
diskbump:
	; Move on a sector
	inc	@tmp4
	bne	bdone
	inc	@tmp4+1
	bne	bdone
	inc	@tmp5
bdone:	pla
	; Returns status
	rts

;
;	Turn a swap slot into a disk LBA
;
slot_to_blk:
	pha
	lda	@tmp4
	; Expand @tmp4 into the offset, for the moment cheat and use 64K
	; chunks and assume swap isn't huge
	; 64K is 128 sectors TODO use proper size
	ldx	#0
	clc
	adc	@tmp4
	clc
	ror	a
	bcc	even
	ldx	#0x80
even:
	; AX is now the offset relative to swap
	pha
	txa
	adc	_swaplba
	tax
	pla
	adc	_swaplba+1
	stx	@tmp4
	sta	@tmp4+1
	lda	_swaplba+2
	adc	#0
	sta	@tmp5
	; tmp4-5 is now a 24bit LBA
	rts

	.commondata

	.export _swapdevh
	.export _swaplba

switch_proc_ptr:
	.word	0
fork_proc_ptr:
	.word	0 ; (C type is struct p_tab *) -- address of child process p_tab entry
tmpbuf_ptr:
	.word	0
; Swap info .. precomputed earlier. We might need to add drive info
; if we support the later dual drive IDE64 TODO
_swaplba:
	.byte	0, 0, 0
_swapdevh:
	.byte	0
