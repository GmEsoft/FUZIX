/*
 *	NemoIDE interface
 *
 *	This is a 16bit interface with a latched data port.
 *
 *	The latch means we have to work from two ports which is ugly but
 *	at least it's not using the high bits of the address.
 */

#define data	0x10
#define error	0x30
#define count	0x50
#define sec	0x70
#define cyll	0x90
#define cylh	0xB0
#define devh	0xD0
#define cmd	0xF0
#define status	0xF0

#define IDE_REG_DATA		0x10
#define IDE_REG_DATA_LATCH	0x11

#define IDE_NONSTANDARD_XFER

#define ide_read(x)	in(x)
#define ide_write(x,y)	out(x,y)
