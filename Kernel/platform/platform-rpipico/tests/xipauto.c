/* Host test for the install-on-first-exec XIP logic.
 *
 * Builds a synthetic static-PIE image, runs the real xip_auto_try() and
 * xip_auto_load() against a RAM-backed flash region, and checks the
 * relocated text in the extent and the relocated data loaded into RAM. The
 * region offset is non-zero, as it is on the board, so a run base that
 * forgets it is caught here.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kernel.h>
#include <kdata.h>
#include <globals.h>
#include <exec.h>

/* ---- host stand-ins for the kernel services xipauto uses ---- */

uint8_t xip_flash[FUZIX_FLASH_XIP_OFFSET + FUZIX_FLASH_XIP_BYTES];
struct u_data udata;
struct p_tab ptab[PTABSIZE];
static uint8_t ram_window[8192];
static unsigned tmpbufs;

#define PROGBASE ((uaddr_t)0x20018000u)
#define PROGLOAD ((uaddr_t)(uintptr_t)ram_window)

static const uint8_t *elf;
static uint32_t elf_len;

void readi(inoptr ino, int flag)
{
	(void)ino;
	(void)flag;
	assert(udata.u_offset + udata.u_count <= elf_len);
	memcpy(udata.u_base, elf + udata.u_offset, udata.u_count);
	udata.u_done = udata.u_count;
}

void *tmpbuf(void)
{
	tmpbufs++;
	return calloc(1, 512);
}

void tmpfree(void *p)
{
	tmpbufs--;
	free(p);
}

void ptimer_insert(void) {}
void psleep(void *event) { (void)event; }

int rpipico_flash_program(uint32_t off, const uint8_t *data, uint32_t len)
{
	uint32_t i;

	assert((off & (FUZIX_FLASH_PAGE_BYTES - 1)) == 0);
	assert((len & (FUZIX_FLASH_PAGE_BYTES - 1)) == 0);
	assert(off + len <= sizeof(xip_flash));
	for (i = 0; i < len; i++) {
		assert((xip_flash[off + i] & data[i]) == data[i]);	/* NOR */
		xip_flash[off + i] &= data[i];
	}
	return 0;
}

int rpipico_flash_erase(uint32_t off, uint32_t len)
{
	assert((off & (FUZIX_FLASH_ERASE_BYTES - 1)) == 0);
	assert((len & (FUZIX_FLASH_ERASE_BYTES - 1)) == 0);
	assert(off + len <= sizeof(xip_flash));
	memset(xip_flash + off, 0xff, len);
	return 0;
}

#include "../xipauto.c"

/* ---- synthetic static-PIE image ---- */

#define TEXTLEN 0x3000u			/* >= XA_MIN_TEXT */
#define DATALEN 0x30u
#define BSS 0x20u
#define DATAOFF (0x100u + TEXTLEN)
#define DATA_VADDR TEXTLEN
#define DYN_VADDR (DATA_VADDR + 0x18u)
#define REL_VADDR 0x1000u
#define RELCOUNT 4

static uint8_t image[0x100 + TEXTLEN + DATALEN];

static void put32(uint32_t off, uint32_t v)
{
	memcpy(image + off, &v, 4);
}

static void put16(uint32_t off, uint16_t v)
{
	memcpy(image + off, &v, 2);
}

static void phdr(unsigned i, uint32_t type, uint32_t off, uint32_t vaddr,
		 uint32_t filesz, uint32_t memsz)
{
	uint32_t b = 52 + i * 32;

	put32(b + 0, type);
	put32(b + 4, off);
	put32(b + 8, vaddr);
	put32(b + 12, vaddr);
	put32(b + 16, filesz);
	put32(b + 20, memsz);
	put32(b + 24, type == 1 ? 7 : 6);
	put32(b + 28, 4);
}

static void build(void)
{
	memset(image, 0, sizeof(image));
	image[0] = 0x7f;
	image[1] = 'E';
	image[2] = 'L';
	image[3] = 'F';
	put16(16, 3);			/* ET_DYN */
	put16(18, 40);			/* EM_ARM */
	put32(20, 1);
	put32(24, 0x101);		/* entry, inside the text load */
	put32(28, 52);			/* phoff */
	put16(40, 52);			/* ehsize */
	put16(42, 32);			/* phentsize */
	put16(44, 3);			/* phnum */
	phdr(0, 1, 0x100, 0, TEXTLEN, TEXTLEN);
	phdr(1, 1, DATAOFF, DATA_VADDR, DATALEN, DATALEN + BSS);
	phdr(2, 2, DATAOFF + 0x18, DYN_VADDR, 0x18, 0x18);

	/* Relocation slots: two in text, two in data. */
	put32(0x100 + 0x100, 0x200);		/* points at text */
	put32(0x100 + 0x200, DATA_VADDR + 8);	/* points at data */
	put32(DATAOFF + 0x0, 0x300);		/* points at text */
	put32(DATAOFF + 0x4, DATA_VADDR + 0x10);	/* points at data */

	put32(DATAOFF + 0x18, 17);		/* DT_REL */
	put32(DATAOFF + 0x1c, REL_VADDR);
	put32(DATAOFF + 0x20, 18);		/* DT_RELSZ */
	put32(DATAOFF + 0x24, RELCOUNT * 8);
	put32(DATAOFF + 0x28, 0);		/* DT_NULL */

	put32(0x100 + REL_VADDR + 0x0, 0x100);
	put32(0x100 + REL_VADDR + 0x4, 23);	/* R_ARM_RELATIVE */
	put32(0x100 + REL_VADDR + 0x8, 0x200);
	put32(0x100 + REL_VADDR + 0xc, 23);
	put32(0x100 + REL_VADDR + 0x10, DATA_VADDR);
	put32(0x100 + REL_VADDR + 0x14, 23);
	put32(0x100 + REL_VADDR + 0x18, DATA_VADDR + 4);
	put32(0x100 + REL_VADDR + 0x1c, 23);
}

static uint32_t rd32(const void *p)
{
	uint32_t v;

	memcpy(&v, p, 4);
	return v;
}

static uint32_t hash(void)
{
	uint32_t h = 2166136261u;
	unsigned i;

	for (i = 0; i < sizeof(xip_flash); i++)
		h = (h ^ xip_flash[i]) * 16777619u;
	return h;
}

int main(void)
{
	struct cinode ino;
	struct exec_xip img;
	uint32_t run, dst, after;
	int idx = -1, r;

	build();
	elf = image;
	elf_len = sizeof(image);
	memset(xip_flash, 0xff, sizeof(xip_flash));
	memset(&ino, 0, sizeof(ino));
	ino.c_dev = 1;
	ino.c_num = 2;
	ino.c_node.i_mtime = 100;
	ino.c_node.i_size = sizeof(image);
	udata.u_ptab = &ptab[0];

	r = xip_auto_try(&ino, image, 0x8000, &img, &idx);
	assert(r == 1);
	assert(tmpbufs == 0);
	assert(img.data_size == DATALEN);
	assert(img.bss_size == BSS);

	run = FUZIX_FLASH_FLASH_BASE + FUZIX_FLASH_XIP_OFFSET + XA_HDR;
	assert(img.entry == run + 0x101);

	/* Relocated text in the extent: a text target keeps the flash run, a
	 * data target becomes a RAM window address. */
	dst = FUZIX_FLASH_XIP_OFFSET + XA_HDR;
	assert(rd32(xip_flash + dst + 0x100) == run + 0x200);
	assert(rd32(xip_flash + dst + 0x200) == (uint32_t)PROGLOAD + 8);

	dst += (TEXTLEN + 511u) & ~511u;
	assert(rd32(xip_flash + dst + 0) == run + 0x300);
	assert(rd32(xip_flash + dst + 4) == (uint32_t)PROGLOAD + 0x10);

	after = hash();
	idx = -1;
	r = xip_auto_try(&ino, image, 0x8000, &img, &idx);
	assert(r == 1 && idx == 0);
	assert(hash() == after);		/* table hit writes nothing */

	/* Pretend to reboot: the table is rebuilt from the region. */
	ready = 0;
	idx = -1;
	r = xip_auto_try(&ino, image, 0x8000, &img, &idx);
	assert(r == 1 && idx == 0 && ntab == 1);
	assert(hash() == after);

	/* Data is loaded relocated and the BSS tail is zeroed. */
	xip_auto_load(idx);
	assert(rd32(ram_window + 0) == run + 0x300);
	assert(rd32(ram_window + 4) == (uint32_t)PROGLOAD + 0x10);
	assert(ram_window[DATALEN] == 0 && ram_window[DATALEN + BSS - 1] == 0);

	/* Only the text extent is readable as a user pointer. */
	assert(udata.u_codebase == run);
	assert(xip_auto_readable(img.entry, 32) == 32);
	assert(xip_auto_readable(img.entry + TEXTLEN, 32) == 0);

	printf("xipauto: host test passed\n");
	return 0;
}
