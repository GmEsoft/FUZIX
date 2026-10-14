# 1 "../platform-smallz80/smallz80.S"
;
;	SmallZ80 support
;
;	This first chunk is mostly boilerplate to adjust for each
;	system.
;

	; exported symbols
	.export init_early
	.export init_hardware
	.export _program_vectors
	.export plt_interrupt_all
	.export _kernel_flag
	.export _int_disabled

	; exported debugging tools
	.export _plt_monitor
	.export _plt_reboot
	.export outchar
# 1 "../platform-smallz80/kernelu.def"
;	FUZIX mnemonics for memory addresses etc
;
;
;	The U_DATA address. If we are doing a normal build this is the start
;	of common memory. We do actually have a symbol for udata so
;	eventually this needs to go away
;
U_DATA__TOTALSIZE           .equ 0x200        ; 256+256 bytes @ F000
;
;	For special platforms that have external memory protection hardware
;	Just say 0.
;


PROGBASE		    .equ 0x4000
PROGLOAD		    .equ 0x4000

U_DATA_STASH		    .equ 0xBE00	      ; BE00-BFFF
Z180_IO_BASE		    .equ 0x00
;
;	The number of disk buffers. Must match config.h
;
NBUFS			    .equ 5
# 1 "../platform-smallz80/../../cpu-z80u/kernel-z80.def"
 
# 26
 
# 44
 
# 1 "../platform-smallz80/../../cpu-z180/z180.def"
; ASCI serial ports
ASCI_CNTLA0                 .equ Z180_IO_BASE+0x00     ; ASCI control register A channel 0
ASCI_CNTLA1                 .equ Z180_IO_BASE+0x01     ; ASCI control register A channel 1
ASCI_CNTLB0                 .equ Z180_IO_BASE+0x02     ; ASCI control register B channel 0
ASCI_CNTLB1                 .equ Z180_IO_BASE+0x03     ; ASCI control register B channel 0
ASCI_STAT0                  .equ Z180_IO_BASE+0x04     ; ASCI status register    channel 0
ASCI_STAT1                  .equ Z180_IO_BASE+0x05     ; ASCI status register    channel 1
ASCI_TDR0                   .equ Z180_IO_BASE+0x06     ; ASCI transmit data reg, channel 0
ASCI_TDR1                   .equ Z180_IO_BASE+0x07     ; ASCI transmit data reg, channel 1
ASCI_RDR0                   .equ Z180_IO_BASE+0x08     ; ASCI receive data reg,  channel 0
ASCI_RDR1                   .equ Z180_IO_BASE+0x09     ; ASCI receive data reg,  channel 0
ASCI_ASEXT0                 .equ Z180_IO_BASE+0x12     ; ASCI extension register channel 0
ASCI_ASEXT1                 .equ Z180_IO_BASE+0x13     ; ASCI extension register channel 1
ASCI_ASTC0L                 .equ Z180_IO_BASE+0x1A     ; ASCI time constant register channel 0 low
ASCI_ASTC0H                 .equ Z180_IO_BASE+0x1B     ; ASCI time constant register channel 0 high
ASCI_ASTC1L                 .equ Z180_IO_BASE+0x1C     ; ASCI time constant register channel 1 low
ASCI_ASTC1H                 .equ Z180_IO_BASE+0x1D     ; ASCI time constant register channel 1 high

; Z180 MMU
MMU_CBR                     .equ Z180_IO_BASE+0x38     ; common1 base register
MMU_BBR                     .equ Z180_IO_BASE+0x39     ; bank base register
MMU_CBAR                    .equ Z180_IO_BASE+0x3A     ; common/bank area register

; Z180 DMA engine
DMA_SAR0L                   .equ Z180_IO_BASE+0x20     ; DMA source address reg, channel 0L
DMA_SAR0H                   .equ Z180_IO_BASE+0x21     ; DMA source address reg, channel 0H
DMA_SAR0B                   .equ Z180_IO_BASE+0x22     ; DMA source address reg, channel 0B
DMA_DAR0L                   .equ Z180_IO_BASE+0x23     ; DMA dest address reg,   channel 0L
DMA_DAR0H                   .equ Z180_IO_BASE+0x24     ; DMA dest address reg,   channel 0H
DMA_DAR0B                   .equ Z180_IO_BASE+0x25     ; DMA dest address reg,   channel 0B
DMA_BCR0L                   .equ Z180_IO_BASE+0x26     ; DMA byte count reg,     channel 0L
DMA_BCR0H                   .equ Z180_IO_BASE+0x27     ; DMA byte count reg,     channel 0H
DMA_MAR1L                   .equ Z180_IO_BASE+0x28     ; DMA memory address reg, channel 1L
DMA_MAR1H                   .equ Z180_IO_BASE+0x29     ; DMA memory address reg, channel 1H
DMA_MAR1B                   .equ Z180_IO_BASE+0x2A     ; DMA memory address reg, channel 1B
DMA_IAR1L                   .equ Z180_IO_BASE+0x2B     ; DMA I/O address reg,    channel 1L
DMA_IAR1H                   .equ Z180_IO_BASE+0x2C     ; DMA I/O address reg,    channel 1H
DMA_BCR1L                   .equ Z180_IO_BASE+0x2E     ; DMA byte count reg,     channel 1L
DMA_BCR1H                   .equ Z180_IO_BASE+0x2F     ; DMA byte count reg,     channel 1H
DMA_DSTAT                   .equ Z180_IO_BASE+0x30     ; DMA status register
DMA_DMODE                   .equ Z180_IO_BASE+0x31     ; DMA mode register
DMA_DCNTL                   .equ Z180_IO_BASE+0x32     ; DMA/WAIT control register

; Z180 Timer
TIME_TMDR0L                 .equ Z180_IO_BASE+0x0C     ; Timer data register,    channel 0L
TIME_TMDR0H                 .equ Z180_IO_BASE+0x0D     ; Timer data register,    channel 0H
TIME_RLDR0L                 .equ Z180_IO_BASE+0x0E     ; Timer reload register,  channel 0L
TIME_RLDR0H                 .equ Z180_IO_BASE+0x0F     ; Timer reload register,  channel 0H
TIME_TCR                    .equ Z180_IO_BASE+0x10     ; Timer control register
TIME_TMDR1L                 .equ Z180_IO_BASE+0x14     ; Timer data register,    channel 1L
TIME_TMDR1H                 .equ Z180_IO_BASE+0x15     ; Timer data register,    channel 1H
TIME_RLDR1L                 .equ Z180_IO_BASE+0x16     ; Timer reload register,  channel 1L
TIME_RLDR1H                 .equ Z180_IO_BASE+0x17     ; Timer reload register,  channel 1H
TIME_FRC                    .equ Z180_IO_BASE+0x18     ; Timer Free running counter

; CSIO
CSIO_CNTR		    .equ Z180_IO_BASE+0x0A     ; CSIO control/status register
CSIO_TRDR		    .equ Z180_IO_BASE+0x0B     ; CSIO transmit/receive data register

; Z180 Interrupts
INT_IL                      .equ Z180_IO_BASE+0x33     ; Interrupt vector low register
INT_ITC                     .equ Z180_IO_BASE+0x34     ; Interrupt vector low register

; Refresh control
MEM_RCR			    .equ Z180_IO_BASE+0x36	; Refresh control

; ESCC serial ports (Z80182)
ESCC_CTRL_A                 .equ 0xE0                   ; ESCC Channel A control register
ESCC_DATA_A                 .equ 0xE1                   ; ESCC Channel A data register
ESCC_CTRL_B                 .equ 0xE2                   ; ESCC Channel B control register
ESCC_DATA_B                 .equ 0xE3                   ; ESCC Channel B data register

PORT_A_DDR                  .equ 0xED                   ; Port A data direction register
PORT_A_DATA                 .equ 0xEE                   ; Port A data register
PORT_B_DDR                  .equ 0xE4                   ; Port B data direction register
PORT_B_DATA                 .equ 0xE5                   ; Port B data register
PORT_C_DDR                  .equ 0xDD                   ; Port C data direction register
PORT_C_DATA                 .equ 0xDE                   ; Port C data register

Z182_SYSCONFIG              .equ 0xEF                   ; System Configuration Register
Z182_RAMUBR                 .equ 0xE6                   ; RAM upper boundary register
Z182_RAMLBR                 .equ 0xE7                   ; RAM lower boundary register
Z182_ROMBR                  .equ 0xE8                   ; ROM boundary register

; Debugging
DEBUGBANK   .equ 0
DEBUGCOMMON .equ 0
# 26 "../platform-smallz80/smallz80.S"
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.export _bufpool
	.buffers

_bufpool:
	.ds 520  * NBUFS

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (kept even when we task switch)
; -----------------------------------------------------------------------------
	.commondata

;
;	Interrupt flag. This needs to be in common memory for most memory
;	models. It starts as 1 as interrupts start off.
;
_int_disabled:
	.byte	1

	.common
;
;	This method is invoked early in interrupt handling before any
;	complex handling is done. It's useful on a few platforms but
;	generally a ret is all that is needed
;
plt_interrupt_all:
	ret

;
;	If you have a ROM monitor you can get back to then do so, if not
;	fall into reboot.
;
;	Wait for a key as the monitor does a screen clear
;
_plt_monitor:
	ld	a,(_uart_base + 1)
	add	a,5
	ld	c,a
pmwait:
	in	a,(c)
	rrca
	jr	nc, pmwait
	; Fall through
;
;	Reboot the system if possible, halt if not. On a system where the
;	ROM promptly wipes the display you may want to delay or wait for
;	a keypress here (just remember you may be interrupts off, no kernel
;	mapped so hit the hardware).
;
_plt_reboot:
	di
	ld	a,(_sys_cpu)
	cp	6			; Z180 ?
	jr	nz, gpio_reboot
	ld	a,(_uart_base + 1)
	add	a,4
	ld	c,a
	xor	a
	; Ensure the ROM is off in all forms, and hit the switch
	out	(c),a
	.z180
	out0	(MMU_BBR),a
	.z80
	rst	0
gpio_reboot:
	out	(0xF8),a
	rst	0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	.code
;
;	This routine is called very early, before the boot code shuffles
;	things into place. We do the ttymap here mostly as an example but
;	even that really ought to be in init_hardware.
;
init_early:
	in a,(0xF8)
	or 0x70		; All lights on
	out (0xF8),a
	ret

; -----------------------------------------------------------------------------
; DISCARD is memory that will be recycled when we exec init
; -----------------------------------------------------------------------------
	.discard
;
;	After the kernel has shuffled things into place this code is run.
;	It's the best place to breakpoint or trace if you are not sure your
;	kernel is loading and putting itself into place properly.
;
;	It's required jobs are to set up the vectors, ramsize (total RAM),
;	and procmem (total memory free to processs), as well as setting the
;	interrupt mode but *not* enabling interrupts. Many platforms also
;	program up support hardware like PIO and CTC devices here.
;
init_hardware:

	ld	a,(0xFFC2)	; Where is the primary serial ?
	cp	0x10
	ld	a, 0x20		; For RTC refs
	jr	z, old_board
	ld	a, 0x50
old_board:
	ld	(_rtc_base),a	; Set RTC infop
	add	a,0x0D		; set up pointer for RTC config
	ld	c,a

	xor	a
	out	(c),a		; hold off, weirdness off
	ld	a,0x02
	inc	c
	out	(c),a		; 64ms, clear any irq
				; irq mode, unmasked
	ld	a,0x04
	inc	c
	out	(c),a		; test off, 24hr mode, stop/reset off

	ld	hl, 0xFFC2	; set up the uart map early
	ld	de, _uart_base + 1

	ldi
	inc	hl
	ldi
	inc	hl
	ldi
	inc	hl
	ldi
	inc	hl
	ld	a,(hl)
	ld	(_num_banks),a
	;	Ignore the expansion board info for now (floppy etc)

	cp	30	; 	30 banks is the Z180 board
	jr	nz, is_z80
	.z180
	ld	a,0xE0
	out0	(MMU_CBAR),a
	.z80
	ld	hl,1024
	ld	(_ramsize), hl
	ld	hl,960
	ld	(_procmem), hl
	jr	setup
is_z80:
	ld	hl,544			; 512 + 32
	ld	(_ramsize), hl
	ld	hl,480
	ld	(_procmem), hl
setup:
	; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
	ld	hl, 0
	push	hl
	call	_program_vectors
	pop	hl

	im 1 ; set CPU interrupt mode

	ret

;
;	Bank switching unsurprisingly must be in common memory space so it's
;	always available.
;
	.commondata

_kernel_flag:
	.byte	1	; We start in kernel mode

	.common
;
;	This is invoked with a NULL argument at boot to set the kernel
;	vectors and then elsewhere in the kernel when the kernel knows
;	a bank may need vectors writing to it.
;
;	FIXME: do this once early as we don't switch the low 16K
;
_program_vectors:
	; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	di ; just to be sure
	pop	de ; temporarily store return address
	pop	hl ; function argument -- base page number
	push	hl ; put stack back as it was
	push	de

	call	map_proc

	; now install the interrupt vector at 0x0038
	ld	a, 0xC3 ; JP instruction
	ld	(0x0038), a
	ld	hl, interrupt_handler
	ld	(0x0039), hl

	ld	(0x0000), a
	ld	hl, null_handler   ;   to Our Trap Handler
	ld	(0x0001), hl

	jp	map_kernel

	;
	; Used for low level debug. Output the character in A without
	; corrupting other registers. May block. Interrupts and memory
	; state are undefined
	;
outchar:
	push	af
	push	bc
	ld	a, (_uart_base + 1)
	add	a,5
	ld	c,a
twait:	in	a, (c)
	bit	5, a
	jr	z, twait
	ld	a,c
	sub	5
	ld	c,a
	pop	af
	out	(c), a
	pop	bc
	ret
