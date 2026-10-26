;
;	SPI interface for the 65SPI on the NX32 with SPI
;

	.export _spi_setup
	.export _sd_spi_slow
	.export _sd_spi_fast
	.export _sd_spi_lower_cs
	.export _sd_spi_raise_cs
	.export _sd_spi_tx_byte
	.export _sd_spi_rx_byte
	.export _sd_spi_rx_sector
	.export _sd_spi_tx_sector

#include "kernel.def"
#include "../../cpu-6809/kernel09.def"

; 65SPI control flags
FRX	equ 0x10		; Fast Receive mode
ECE	equ 0x04		; External Clock Enable

	.code

_spi_setup:
	clra
	sta	SPICTRL		; just in case
	lda	#0x0F
	sta	SPISIE		; selects off, IRQs off
	cmpa	SPISIE
	bne	nospi
	lda	SPIDATA		; clear TC
	lda	SPISTATUS
	bmi	nospi 		; TC not clear -> no spi present
	sta	SPIDATA		; start a transmit, TC should now be clear
	lda	SPISTATUS
	bpl	spigood
nospi:	clrb
	rts
spigood:
	ldb	#1
	rts

;
;	May need to switch clocks too

_sd_spi_fast:
	; external 45MHz clock on, divide by 4
	ldd	#ECE*256+0x01
	std	SPICTRL
	rts

_sd_spi_slow:
	; internal clock, phi/2 -> 0.89MHz/2 = 445kHz
	clra
	clrb
clkset:	std	SPICTRL
	rts

;
;	For multiple cards these need to look at the card #
;
_sd_spi_raise_cs:
	lda	SPISIE
	ora	#SPICS
	sta	SPISIE
	rts

_sd_spi_lower_cs:
	lda	SPISIE
	anda	#0xFF-SPICS
	sta	SPISIE
	rts

;
;	We must check for busy as these methods are used at low speed
;
_sd_spi_tx_byte:
	stb	SPIDATA
txwait:
	lda	SPISTATUS
	anda	#0x20		; BSY
	bne	txwait
	rts

_sd_spi_rx_byte:
	lda	#0xFF
	sta	SPIDATA
rxwait:
	lda	SPISTATUS	; wait for bit 7 to set
	bpl	rxwait
	ldb	SPIDATA
	rts

	.common
;
;	The SPI clock must be in fast mode here as we don't poll the busy
;	bits but know that by the time the CPU arrives at the next byte
;	the transaction is done. Note - we don't have to worry about the
;	interrupt situation here, we drive the clocks so if we go off for
;	an interrupt all is fine.
;
spisetup:
	tfr	d,x
	leay	512,x
	sty	endspi
	lda	#0xFF
	tfr	a,dp
	lda	_td_raw
#ifndef CONFIG_SWAP
	lbne	map_proc_always
	rts
#else
	bne	domap
	rts
domap:
	cmpa	#1
	lbeq	map_proc_always
	lda	_td_page
	jmp	map_proc_a
#endif

_sd_spi_rx_sector:
	pshs	dp
	bsr	spisetup
	lda	#ECE+FRX		; FRX on, external clock on
	sta	@SPICTRL
	lda	@SPIDATA		; read old value, triggers shifting in new
read8:
	lda	@SPIDATA
	ldb	@SPIDATA
	std	,x++
	lda	@SPIDATA
	ldb	@SPIDATA
	std	,x++
	lda	@SPIDATA
	ldb	@SPIDATA
	std	,x++
	lda	@SPIDATA
	ldb	@SPIDATA
	std	,x++
	cmpx	endspi
	bne	read8
	jsr	map_kernel
	lda	#ECE
	sta	@SPICTRL		; FRX off, external clock on
	puls	dp,pc

_sd_spi_tx_sector:
	pshs	dp
	bsr	spisetup
	lda	#ECE		; ext clock, no FRX
	sta	SPICTRL
write8:
	ldd	,x++
	sta	@SPIDATA
	stb	@SPIDATA
	ldd	,x++
	sta	@SPIDATA
	stb	@SPIDATA
	ldd	,x++
	sta	@SPIDATA
	stb	@SPIDATA
	ldd	,x++
	sta	@SPIDATA
	stb	@SPIDATA
	cmpx	endspi
	bne	write8
	jsr	map_kernel
	puls	dp,pc

	.commondata

endspi:	.word 0
