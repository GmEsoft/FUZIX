;
;	6809 copy to and from userspace
;
;	This is linked by the 6881 using platforms only. MMU based
;	machines simply don't need it.
;
;	FIXME: IRQ should not need to be off
;
#include "kernel.def"
#include "../../cpu-6809/kernel09.def"

	; exported
	.export __ugetc
	.export __ugetw
	.export __uget

	.export __uputc
	.export __uputw
	.export __uput
	.export __uzero

	.common

__ugetc:
	pshs	cc		; save IRQ state
	tfr	d,x
	orcc	#0x10
	jsr	map_proc_always
	ldb	,x
	jsr	map_kernel
	clra
	puls	cc,pc		;back and return

__ugetw:
	tfr	d,x
	pshs	cc
	orcc	#0x10
	jsr	map_proc_always
	ldd	,x
	jsr	map_kernel
	puls	cc,pc

__uget:
	pshs	u,cc
	tfr	d,x
	ldu	5,s		; user address
	ldy	7,s		; count
	orcc	#0x10
ugetl:
	jsr	map_proc_always
	lda	,x+
	jsr	map_kernel
	sta	,u+
	leay	-1,y
	bne	ugetl
	clra
	clrb
	puls	u,cc,pc

	; FIXME: ldb 4,s ?
__uputc:
	pshs	cc
	orcc	#0x10
	ldx	3,s
	jsr	map_proc_always
	stb	,x
	jsr	map_kernel
	clra
	clrb
	puls	cc,pc

__uputw:
	pshs	cc
	orcc	#0x10
	ldx	3,s
	jsr	map_proc_always
	std	,x
	jsr	map_kernel
	clra
	clrb
	puls	cc,pc

;	D = source, user, size on stack
__uput:
	pshs	u,cc
	orcc	#0x10
	tfr	d,x
	ldu	5,s		; user address
	ldy	7,s		; count
uputl:
	lda	,x+
	jsr	map_proc_always
	sta	,u+
	jsr	map_kernel
	leay	-1,y
	bne	uputl
	clra
	clrb
	puls	u,cc,pc

__uzero:
	pshs	cc
	ldy	3,s
	orcc	#0x10
	jsr	map_proc_always
	tfr	d,x
	tfr	y,d
	clra
	lsrb		; odd count?
	bcc	evenc
	sta	,x+
	leay	-1,y
	beq	zdone
evenc:
	clrb
uzloop:
	std	,x++
	leay	-2,y
	bne	uzloop
zdone:
	jsr	map_kernel
	clra
	clrb
	puls	cc,pc
