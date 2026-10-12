# Atari 8bit

Initial work on an 8bit Atari port. Still writing the driver pieces and
will need to redo tricks.S big time for 6502 single task in memory (and
some of that is tricky see below)

Runs from a 128K SpartaDOS style cartridge

Memory Map:
0000-00FF	Zero Page (shared between kernel and user)
0100-01FF	Stack Page (shared between kernel and user)
0200-03FF	Space for initial boot
0400-3FFF	Code and data discarded after initial booting
4000-7FFF	Banked (4 overlays and user)
8000-9FFF	User
A000-BFFF	User or Kernel code from cartridge directly
C000-CFFF	Bits of shared space
D000-D7FF	I/O
D800-FFFF	Kernel data/bss/writable common

When in user space our map ends up
0000-00FF	Zero Page (shared between kernel and user)
0100-01FF	Stack Page (shared between kernel and user)
0200-03FF	User
0400-3FFF	User
4000-7FFF	User
8000-9FFF	User
A000-BFFF	User 
C000-CFFF	Bits of shared space
D000-D7FF	I/O
D800-FFFF	Kernel data/bss/writable common

We hide a lot in an overlay segment overlay4. However we use extended
memory for the user process and main memory for overlay4 as we keep the
video in this 16K block

Overlay4 is carefully laid out as

4000		Font
4400		Video space (8K)
5400		Disk buffers (2.5K)
~5800		Code for overlay 4

This gives us enough space for the full video mode range in future and keeps
everything out of the user map. We keep the video in the main bank so we can
handle less functional RAMBO banking in future, and also AXLON style banks.

## Memory Banking

Currently we only support RAMBO style banking. We don't use the ability to
put ANTIC fetches into the extra banks so this should work with many of the
extended variants people produced).

Supporting AXLON style banking isn't particularly tricky. The 16K pages are
the same with page 0 as the base memory. The paging mechanism however means
we have to be very careful about 0FC0-0FFF and CFC0-CFFF. CFC0-CFFF is easy
to deal with. AXLON banking is demented as the bank codes differ between
boards so there's no trivial mapping to know what values work. AXLON style
banking however would mess up the video as the video tracks the CPU
banking generally. We'd be limited to say a 40x25 text screen.

There is also the better but somewhat obscure Byte design that switches the
low 32K based on PORTB 4-6 (bank 6 being the upper 32K)

RAMBO style banking above the 4 banks gets messy and it's not clear we can
make good use of extra anyway. AXLON banking goes up to 1MB and sometimes
4MB which would be sufficient to use as your swap device not the disk. If
so it would also save 16K of copy in/out for the 4000-7FFF range

## Constraints

### C000-CFFF

This is not present on old machines. Do we care, could we pack stuff to
handle this better

### CFC0-CFFF / 0FC0-0FFF

Writes to these blocks do AXLON style bank switching. For the 0Fxx alias this
could be problematic. One option would be to move discard up and keep the
common at 0200-1000 or so and put non writeable common space there. User
would then extend up to CFBF or BFFF depending upon the machine and presence
of the 4K at C000.

### Video

Our current setup uses the main bank at 4000 for video but some AXLON style
cards will feed ANTIC whatever bank the CPU is using. Not clear we can do
much with such cards.

### Problems To Solve
ZP we can handle between the process and kernel by splitting the allocation.
Need a special disk loader for ZP for swap that only stores some of the
stack so we have a bit of space for swap (say top 32 bytes with careful
management)

S is a real problem. User space is fine, syscall is fine as it will use the
same stack. Might need an Apple like "stack is a bit deep copy/fix/replace"
helper but we can do that fine.

The problems are context switches and swapping

Possible option
- Custom 'read page 0' routine in disk swap driver

Load 00-EF skip F0-FF as workspace for swapper asm
Load 100-1BF as ustack and kstack. preserve 1C0-1FF (maybe less) as swap
stack. We'd still have the separate C swap and main stacks too

So something like

```
next1:
	lda	diskdata
	sta	(@swapptr),y
	iny
	cpy	#0xF0
	bne	next1
disc1	lda	diskdata
	iny
	bne	disc1
	inc	@swapptr+1
next2:	lda	diskdata
	sta	(@swapptr),y
	iny
	cpy	#0xC0
	bne	next2
disc2	lda	diskdata
	iny
	bne	disc2
```

With a big RAM card we can use the extra pages of extra RAM as swap as we've
got only the 16K window but it would be more optimal to copy 32K and treat
is as banked as that cuts our copy down by 16K.
