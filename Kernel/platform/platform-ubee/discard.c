#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <devhd.h>
#include <ubee.h>

#define CMOS_REG	0x04
#define CMOS_READ	0x07

extern int strcmp(const char *, const char *);

void has_cmos_rtc(void)
{
	/* See if the week looks valid - probably want a stronger check */
	out(CMOS_REG, 0x06);
	if (in(CMOS_READ) == 0 || in(CMOS_READ) > 7)
		panic("RTC required");
}

void map_init(void)
{
}

#define FDC_CYL		0x41
#define PROBE_REG	0x45
#define FDC_STAT	0x48
#define PORTSWITCH	0x58

void do_diskprobe(uint8_t *p)
{
	out(PROBE_REG, 0x5A);
	/* probe_reg is read/write same value for both controllers so if it
	   doesn't work we have no disk */
	if (in(PROBE_REG) != 0x5A)
		return;
	/* If it mirrored check if a second value mirrors */
	if (in(FDC_CYL) == 0x5A) {
		out(PROBE_REG, 0x22);
		if (in(FDC_CYL) == 0x22) {
			/* It's a floppy */
			if (in(FDC_STAT) & 0x0F)
				*p = DISK_TYPE_FDC_D;	/* Dream disc */
			else
				*p = DISK_TYPE_FDC;
			return;
		}
	}
	/* Double check */
	out(PROBE_REG, 0x22);
	if (in(PROBE_REG) != 0x22)
		return;
	*p = DISK_TYPE_HDC;
}

static const char dcstr[] = "Disk controller %d: %s\n";
static const char *diskname[4] = { NULL, "2793", "DreamDisc", "WD1010" };

void diskprobe(void)
{
	out(PORTSWITCH, 0);
	do_diskprobe(disk_type);
	out(PORTSWITCH, 1);
	do_diskprobe(disk_type + 1);
	/* Hard case: two of the same */
	if (disk_type[0] == disk_type[1]) {
		/* No disk at all - we don't care if port 58 works */
		if (!disk_type[0])
			return;
		/* We might have internal and external the same ? */
		out(PROBE_REG, 0x5C);
		out(PORTSWITCH, 0);
		out(PROBE_REG, 0x00);
		out(PORTSWITCH, 1);
		if (in(PROBE_REG) != 0x5C)
			disk_type[1] = 0;
	}
	kprintf(dcstr, 0, diskname[disk_type[0]]);
	if (disk_type[1])
		kprintf(dcstr, 1, diskname[disk_type[1]]);
}

/* The bank bits are not laid out nicely for historical reasons

   A classic uBee has the bank selection in bits 0/1
   The 256TC needed an extra bit so reused bit 5 (rom selector)
   The 3rd party 512K/1MB add ons used bits 7/6 for the high bank bits

   The actual location of the high memory is physically 0/0, however bit
   1 gets inverted when ROM is not mapped so we must avoid what we think
   of as page 2 */

static uint8_t map_mod(uint8_t banknum)
{
	uint8_t r = banknum & 0x03;
	r |= 0x14;		/* ROM off, Video on at 0x8000 */
	if (ubee_model == UBEE_256TC)
		r |= (banknum & 4) ? 0x20 : 0;
	else
		r |= (banknum & 0x0C) << 4;
	return r;
}

/* Kernel in bank 0 low/high,user apps in bank 1 low/high (and if present
   other banks too). Memorywise it's a lot like the TRS80 layout */

void pagemap_init(void)
{
	uint8_t i;
	uint8_t nbank = ramsize / 32;

	/* Just a handy spot to run it early */
	has_cmos_rtc();

	/* Our kernel lives in slot 0 as we see it. Our upper bank depends
	   upon the machine type and is 2. We skip that as we assign pages */
	for (i = 1; i < nbank; i++) {
		if (i == 2)
			continue;
		pagemap_add(map_mod(i));
	}
}

uint_fast8_t plt_param(char *p)
{
	/* Q: do we need to remember we are running at 6.75MHz anywhere ? */
	if (strcmp(p, "turbo") == 0) {
		engage_warp_drive();
		return 1;
	}
	if (strcmp(p, "joystick") == 0) {
		ubee_parallel = UBEE_PARALLEL_JOYSTICK;
		return 1;
	}
	if (strcmp(p, "beetalker") == 0) {
		ubee_parallel = UBEE_PARALLEL_BEETALKER;
		return 1;
	}
	if (strcmp(p, "beethoven") == 0) {
		ubee_parallel = UBEE_PARALLEL_BEETHOVEN;
		return 1;
	}
	if (strcmp(p, "dac") == 0) {
		ubee_parallel = UBEE_PARALLEL_DAC;
		return 1;
	}
	if (strcmp(p, "compumuse") == 0) {
		ubee_parallel = UBEE_PARALLEL_COMPUMUSE;
		return 1;
	}
	return 0;
}

void device_init(void)
{
  /* Time of day clock */
  inittod();
  /* Figure out what disks we have */
  diskprobe();
#ifdef CONFIG_IDE
  /* IDE */
  devide_init();
#endif
  /* ST506 */
  hd_init();
}
