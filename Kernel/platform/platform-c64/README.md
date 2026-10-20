# Commodore 64 + IDE64

This is just any early development test of a simple C64 + IDE64 setup with the
OS in the IDE64 cartridge memory. A more serious setup would be to build
a C64 + IDE64 + REU configuration as we can use the REU fast memory swap
for process switching and we can also use the REU as a fast memory zero,
copy and even in theory disk to REU direct transfer. That would allow us to
keep the buffers in REU space and also read/write the disk really fast in
CF mode.

At this point we can get to starting /init but then things go very downhill
which is not surprising as the 6502 syscall side code needs a fair bit of
work for this kind of memory model. Currently it's running on machines where
we can swap the ZP/stack using memory banking which is *much* easier.

## Memory Map

0000-00FF	Zero Page
0100-01FF	Stack
0200-03FF	C64 Kernal owns
0400-07FF	Screen
(0800-0FFF	font ?)
0800-3FFF	Kernel data/buffers etc
4000-7FFF	Always user
8000-AFFF	Cartridge (banked 4 ways - kernel), RAM user
B000-BFFF	Cartridge (same code in each 4K bank)
C000-FEFF	User, D000-DFFF becoems I/O when cartridge is paged in
E000-FFFF	Becomes kernal when cartridge paged in so icky mess
		with vectors
FF00-FFFF	REU trigger, vectors (maybe put istack here)

## Issues

Pretty much as Atari 8bit. In fact very similar ports except for a few
drivers. See notes on WIP atari 8bit.

Disk I/O requires accessing Dxxx. We have user memory at Dxxx and it's not
possible to have both mapped at once. Therefore like MSX1 we will need to
bounce stuff in that range or keep flipping back and forth for that case
which *might* be better.

### Kernal

We get the system ROM shoved in our face whether we like it or not. It's
not a problem for user (all RAM mode it goes away) but does mean we take
interrupts in kernel mode via the kernal layer so might have to be polite 8(

Kernal requires most of 0200-03FF which isn't too bad and hogs E000-FFFF
in cartridge mapped mode which is actually fine (except for the high vectors
it's all user anyway)

Vectors:
0314/5	IRQ
0316/7	BRK
0318/9	NMI

ZP: Probably safe to use 02 to 8F if avoiding BASIC
90-FF avoid

JSR $FF81 to initialize the VIC
JSR $FF84 to initialize the devices early in cartridge init
JSR $FF8D XY = address, set carry, copies vectors in XY to 0314-0332
A= 0 JSR $FF90 ; shut up kernel error printing
JSR $FFE4 - gets a char from the keyboard queue, does all the work for us if
the IRQs are running.  0 = non char. Need to remap from PETSCII 8(

For RAM side we need to do the following on an IRQ to match kernal

	PHA
	TXA
	PHA
	TYA
	PHA
	TSX
	LDA	0x0104,X
	AND	0x10
	BEQ	IS_IRQ
	JMP	(0316)
IS_IRQ:	JMP	(0314)

Then our handler needs to figure out how to call the provided vector and
wrap it so that our pre-emption doesn't blow stuff up

	PHA
	PHA
	PHA
	PHP		; for the RTI
	JSR	(OLDVEC)	; will pop 3 words and RTI
	JSR	OUR_STUFF
	PLA
	TAY
	PLA
	TAX
	PLA
	RTI



## VICE

-cartide  -IDE64Image1 foo.cfa

Need a 4.x cartridge to get 128K ROM

4.1	Write to 0x60-7 with bank 0-7
4.2+	Write to 0x68+ with bank 0-31
4.x	Write to 0xFF with IO and GAME bits

Raw 128K  8 x 16K or 512K image

(the VICE cartridge conversion tool seems broken for IDE64, raw images work
but crt ones do not)
