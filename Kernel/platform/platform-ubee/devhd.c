/*
 *	The MicroBee 128 shipped with a WD1002-05 and various small drives.
 *
 *	The driver is based on the TRS80 WD1010 driver. The 1002-05 is a 1010
 *	with additional glue chips that also manage and front up a floppy
 *	controller for us including the DMA management. It's actually a better
 *	floppy controller than almost anything that followed it !
 *
 *	Minor is arranged as
 *
 * 	S		slot (0 or 1 via port 0x58)
 *	DDD		drive identifier
 *			0-2	hdc
 *			3-6	fdc
 *	MMMM		0000 (reserved for partitioning on hd)
 *
 *	TODO: switch to tinydisk ?
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <ubee.h>
#include <blkdev.h>

#define	HD_DATA		0x40
#define HD_PRECOMP	0x41	/* W/O */
#define HD_ERR		0x41	/* R/O */
#define HD_SECCNT	0x42
#define HD_SECNUM	0x43
#define HD_CYLLO	0x44
#define HD_CYLHI	0x45
#define HD_SDH		0x46
#define HD_STATUS	0x47	/* R/O */
#define HD_CMD		0x47	/* W/O */
#define HD_FDCSIDE	0x48	/* Side select for FDC */
#define FDC_DEVSEL	0x58	/* Whch controller to use */

#define HDCMD_RESTORE	0x10
#define HDCMD_READ	0x20
#define HDCMD_WRITE	0x30
#define HDCMD_VERIFY	0x40	/* Not on the 1010 later only */
#define HDCMD_FORMAT	0x50
#define HDCMD_INIT	0x60	/* Ditto */
#define HDCMD_SEEK	0x70

#define RATE_2MS	0x04	/* 2ms step rate for hd (conservative) */
#define RATE_6MS	0x06	/* 6ms step for floppy (3ms is probably fine
				   for most) */

#define HDSDH_ECC256		0x80

/* Used by the asm helpers */
uint8_t hd_page;

/* Seek and restore low 4 bits are the step rate which we need to sort out
   FIXME */

#define MAX_HD	3	/* devsel 3 means floppy */
#define MAX_FDC	4

/* Standard formats for Microbee:
	40 x 2 x 10 x 512 double sided 5.25
	80 x 2 x 10 x 512 single sided 3.5
	80 x 2 x 10 x 512 double sided 3.5

	For now just use double sided : FIXME */
static int spt[7] = { 32, 32, 32, 10, 10, 10, 10 };
static int heads[7] = { 4, 4, 4, 2, 2, 2, 2 };

/* Wait for the drive to show ready */
static uint8_t hd_waitready(void)
{
	uint8_t st;
	uint16_t tick = 0;
	do {
		st = in(HD_STATUS);
		tick++;
	} while (!(st & 0x40) && tick);
	return st;
}

/* Wait for the controller to show idle */
static uint8_t hd_waitidle(void)
{
	uint8_t st;
	uint16_t tick = 0;
	do {
		st = in(HD_STATUS);
		tick++;
	} while ((st & 0x80) && tick);
	return st;
}

/*
 *	Transfer for routine for the WD1010 (ST-506) interface.
 */

static uint8_t hd_transfer_sector(void)
{
	uint8_t drive = blk_op.blkdev->driver_data & 0x0F;
	uint8_t sel = !!(drive & 0x08);

	uint32_t b;
	uint8_t sector, head;
	uint8_t err;
	uint8_t tries;

	drive &= 0x07;

	b = blk_op.lba / spt[drive];
	sector = blk_op.lba % spt[drive];
	head = b % heads[drive];
	b /= heads[drive];

	out(FDC_DEVSEL, sel);

	if (drive < MAX_HD) {
		/* ECC, 512 bytes, head and drive */
		out(HD_SDH, 0xA0 | head | (drive << 3));
	} else {
		/* Floppy setup */
		out(HD_FDCSIDE, head);
		/* CRC, 512 bytes, head (0/1), FDC unit, fdc number */
		out(HD_SDH, 0x38 | head | ((drive - MAX_HD) << 1));
	}
	out(HD_SECNUM, sector + 1);

	/* cylinder bits */
	out(HD_CYLLO, b);
	out(HD_CYLHI, b >> 8);

	for (tries = 0; tries < 4; tries++) {
		/* issue the command */
		out(HD_CMD, blk_op.is_read ? HDCMD_READ : HDCMD_WRITE);
		err = 0;
		/* Wait for busy to drop if reading before xfer */
		if (blk_op.is_read) {
			hd_waitidle();
			hd_xfer_in();
			err = in(HD_STATUS);
		} else {
			hd_xfer_out();
			/* Wait for busy to drop after xfer if writing */
			err = hd_waitidle();
			/* Ready, no error, not busy ? */
		}
		if ((err & 0xC1) == 0x40)
			return 1;

		kprintf("hd%d: err %x\n", drive, err);

		if (tries > 1) {
			out(HD_CMD, HDCMD_RESTORE | RATE_6MS);
			if (hd_waitready() & 1)
				kprintf("hd%d: restore error %x\n", drive, err);
		}
	}
	if (err & 1)
		kprintf("hd%d: error %x\n", drive, in(HD_ERR));
	else
		kprintf("hd%d: status %x\n", drive, err);
	return 0;
}

static void hd_init_drive(uint8_t drive)
{
	blkdev_t *blk;
	uint8_t sel = !!(drive & 0x08);

	/* Only probe controllers that are present */
	if (disk_type[sel] != DISK_TYPE_HDC)
		return;

	out(FDC_DEVSEL, sel);

	drive &= 0x07;

	if (drive < MAX_HD) {
		out(HD_SDH, 0xA0 | (drive << 3));
		out(HD_CMD, HDCMD_RESTORE | RATE_2MS);
	} else {
		out(HD_SDH, 0x38 | ((drive - MAX_HD) << 1));
		out(HD_CMD, HDCMD_RESTORE | RATE_6MS);
	}
	if (hd_waitready() & 1) {
		if ((in(HD_ERR) & 0x12) == 0x12)
			return;
	}
	/* Found */
	blk = blkdev_alloc();
	if (blk == NULL) {
		kputs("hd: too many drives.\n");
		return;
	}
	blk->transfer = hd_transfer_sector;
	blk->flush = NULL;
	blk->driver_data = drive | (sel << 3);
	blk->drive_lba_count = 0xFFFFFF;
	/* No partitions on floppy disks thank you */
	if (drive < MAX_HD)
		blkdev_scan(blk, SWAPSCAN);
}

void hd_init(void)
{
	uint8_t d;
	/* 0 1 2 are hard disks, 3-7 floppy appearing as hd */
	/* 8+ because we also need to scan the other slot if present */
	for (d = 0; d < 15; d++)
		hd_init_drive(d);
}

