;
;	Glennside style IDE block transfer logic
;

	.export _devide_read_data
	.export _devide_write_data

#include "kernel.def"
#include "../../cpu-6809/kernel09.def"

	.common

;
;	Standad mapping for Glennside style IDE
;
idesetup:
	tfr	d,x
	leay	512,x
	sty	endp
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
;
;	Not yet supporting swap
;
_devide_read_data:
	pshs	dp
	jsr	idesetup
readword:
	lda	@IDEDATA
	ldb	@IDEDATA_L		; latched
	std	,x++
	cmpx	endp
	bne	readword
	jsr	map_kernel
	puls	dp,pc

_devide_write_data:
	pshs	dp
	jsr	idesetup
writeword:
	ldd	,x++
	stb	@IDEDATA_L
	sta	@IDEDATA
	cmpx	endp
	bne	writeword
	jsr	map_kernel
	puls	dp,pc

endp:	.word	0
