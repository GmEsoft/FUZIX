/* Install-on-first-exec XIP for the RP2040 port.
 *
 * The first time an ordinary static-PIE binary is executed its text is
 * copied into the reserved flash region and it runs in place from there;
 * later execs reuse the installed extent. Extents are appended and
 * self-describing, and the RAM table is rebuilt by scanning them. Anything
 * too small or unsupported loads into RAM.
 *
 * Flash is written only when no other process is runnable and with
 * interrupts disabled across each program or erase, so an install can
 * never overlap live I/O. The region is append-only; a full region falls
 * back to RAM rather than erase flash a running image lives in.
 *
 * Text and data are stored relocated: text words become flash addresses,
 * data words RAM-window addresses, so exec is a plain copy.
 */

#include <kernel.h>
#include <kdata.h>
#include <elf.h>
#include <exec.h>
#include <globals.h>
#include <picosdk.h>

/* Tunables ---------------------------------------------------------- */

/* Reserved flash region, from the generated layout. 4 KiB aligned. */
#define XA_LEN		FUZIX_FLASH_XIP_BYTES
#define XA_ROFF		FUZIX_FLASH_XIP_OFFSET
#define XA_FBASE	FUZIX_FLASH_FLASH_BASE

/* Maximum resident images (the region fits far fewer). */
#define XA_NTABLE	16u

/* Pages per flash park cycle; parking dominates install time and USB
 * survival, so consecutive pages share one cycle. */
#define XA_BATCH	8u

/* Minimum text size worth installing: below this the flash ops, table
 * slot and region consumed outweigh the SRAM saved. */
#define XA_MIN_TEXT	(10u * 1024u)

/* Flash units. */
#define XA_PAGE		512u
#define XA_SECTOR	4096u

/* Extent format ----------------------------------------------------- */

#define XA_MAGIC	0x5455415Bu	/* "XAUT"; bump to invalidate extents */
#define XA_VALID	0xAA55AA55u	/* written last, after the payload */
#define XA_HDR		512u		/* header size == program unit */
#define XA_MAXRANGE	4u		/* data ranges beside the text */

/* One data range: link span, RAM window offset and extent file image. */
struct xa_range {
	uint32_t link, mem, win, off, len;
};

/* Extent header, followed by the payload. Little endian; read with memcpy
 * only. dev/num/mtime/size/pool identify the installed file. */
struct xa_hdr {
	uint32_t magic, valid, total;	/* total extent bytes, page aligned */
	uint16_t dev, num;
	uint32_t mtime, size, pool;	/* pool is PROGBASE at install */
	uint32_t entry_off, text_len, data_len, nranges;
	struct xa_range data[XA_MAXRANGE];
};
_Static_assert(sizeof(struct xa_hdr) <= XA_HDR, "xipauto header overflows a page");

/* Program headers of the file being installed. */
struct xa_load {
	uint32_t vaddr, filesz, memsz, offset;
};

/* The chosen split: text is stored verbatim, data ranges are packed into
 * one contiguous RAM window in load order. */
struct xa_plan {
	uint32_t text_link, text_len, text_file, entry_off;
	unsigned ndata;
	struct xa_range data[XA_MAXRANGE];
};

/* Parse state, kept off the small kernel stack. */
struct xa_parse {
	struct xa_load loads[8];
	struct xa_plan plan;
};
_Static_assert(sizeof(struct xa_parse) <= 512, "xipauto parse state fits tmpbuf");

_Static_assert(FUZIX_FLASH_PAGE_BYTES == XA_PAGE, "xipauto programs 512B pages");
_Static_assert(FUZIX_FLASH_ERASE_BYTES == XA_SECTOR, "xipauto erases 4KiB sectors");
_Static_assert((FUZIX_FLASH_XIP_BYTES & (XA_SECTOR - 1)) == 0, "xipauto region sector sized");
_Static_assert(FUZIX_FLASH_XIP_BYTES != 0, "xipauto needs a flash region");

/* -------------------------------------------------------------------- */

/* Reads use the uncached alias: the XIP cache is stale after programming.
 * Execution addresses use the cached alias, like the rest of the system. */
#define XA_MEM	((const uint8_t *)(uintptr_t)(XIP_NOCACHE_NOALLOC_BASE + XA_ROFF))

static struct xa_hdr tab[XA_NTABLE];	/* rebuilt by the boot scan */
static uint32_t tab_off[XA_NTABLE];	/* region-relative extent offset */
static unsigned ntab;
static uint32_t bump;			/* append point */
static uint32_t erased;			/* first not-known-erased sector */
static int ready;			/* boot scan has run */
static int busy;			/* install in progress */

static uint8_t pbuf[XA_PAGE * XA_BATCH];	/* page staging */
static uint32_t poff;
static unsigned pnum;

/* Cached-alias base of an extent's text run. */
static uint32_t text_run(uint32_t ext)
{
	return XA_FBASE + XA_ROFF + ext + XA_HDR;
}

/* Cached-alias text base of a table entry. */
static uint32_t text_base(unsigned i)
{
	return text_run(tab_off[i]);
}

/* True when no other process is runnable, so flash mutation cannot overlap
 * live I/O. */
static int xa_quiet(void)
{
	unsigned i, n = 0;

	for (i = 0; i < PTABSIZE; i++) {
		uint8_t st = ptab[i].p_status;

		if (st == P_RUNNING || st == P_READY || st == P_FORKING ||
		    st == P_NOSLEEP)
			n++;
	}
	return n <= 1;
}

/* Sleep a few ticks holding nothing. */
static void xa_sleep(int ticks)
{
	irqflags_t irq = di();

	udata.u_ptab->p_timeout = ticks + 1;
	ptimer_insert();
	psleep(0);
	irqrestore(irq);
}

/* Yield one tick so a forking parent can reach wait; the exec hook also
 * uses it to retry a busy install. */
void xip_auto_yield(void)
{
	xa_sleep(1);
}

/* Program the buffered run. A transient wakeup must not scrap a nearly
 * complete install: wait it out briefly. Interrupts are disabled across
 * each check, so no process can wake between it and the flash op. */
static int xa_flush(void)
{
	int tries = 4;

	if (!pnum)
		return 0;
	for (;;) {
		irqflags_t irq = di();
		int q = xa_quiet();

		if (q)
			rpipico_flash_program(poff, pbuf, pnum * XA_PAGE);
		irqrestore(irq);
		if (q)
			break;
		if (--tries == 0)
			return -1;
		xa_sleep(2);
	}
	pnum = 0;
	return 0;
}

/* Staging slot for the page at region offset off, flushing whatever run is
 * pending when this page does not continue it. NULL on a failed flush. */
static uint8_t *xa_slot(uint32_t off)
{
	if (pnum && (pnum == XA_BATCH || off != poff + pnum * XA_PAGE) &&
	    xa_flush())
		return NULL;
	if (!pnum)
		poff = off;
	return pbuf + pnum++ * XA_PAGE;
}

/* Buffer a header page; 0 on failure. */
static int xa_header(uint32_t ext, const struct xa_hdr *h)
{
	uint8_t *pg = xa_slot(XA_ROFF + ext);

	if (!pg)
		return 0;
	memset(pg, 0xff, XA_PAGE);
	memcpy(pg, h, sizeof(*h));
	return 1;
}

/* Erase one sector under the same invariant as programming. */
static int xa_erase_sector(uint32_t off)
{
	irqflags_t irq = di();
	int q = xa_quiet();

	if (q)
		rpipico_flash_erase(off, XA_SECTOR);
	irqrestore(irq);
	return q ? 0 : -1;
}

/* True when the sector already holds only erased bytes. */
static int xa_blank(uint32_t roff)
{
	const uint8_t *p = XA_MEM + roff;
	uint32_t i;

	for (i = 0; i < XA_SECTOR; i++) {
		if (p[i] != 0xff)
			return 0;
	}
	return 1;
}

/* Read file bytes into a kernel buffer (no valaddr dance). A short read
 * abandons the install; the RAM loader then reports the file itself. */
static int xa_read(inoptr ino, uint32_t off, void *buf, usize_t len)
{
	udata.u_offset = off;
	udata.u_count = len;
	udata.u_base = buf;
	udata.u_sysio = true;
	readi(ino, 0);
	return udata.u_done == len ? 0 : -1;
}

/* does data + bss + stack fit below himem? */
static int xa_fits(uint32_t data, uint32_t bss, uaddr_t himem)
{
	uaddr_t top;

	if (data > himem || bss > himem - data)
		return -1;
	top = (uaddr_t)ALIGNUP(data + bss) + USERSTACK;
	return top > himem ? -1 : 0;
}

/* Map a link-time address to its file offset through any load. */
static int xa_map(const struct xa_load *l, unsigned n, uint32_t vaddr,
		  uint32_t *off)
{
	unsigned i;

	for (i = 0; i < n; i++) {
		if (vaddr - l[i].vaddr < l[i].filesz) {
			*off = l[i].offset + (vaddr - l[i].vaddr);
			return 0;
		}
	}
	return -1;
}

/* Plan the split: the entry selects the text load, which must have no
 * trailing BSS; every other non-empty load is a data range, packed into
 * one contiguous RAM window in load order. */
static int xa_plan(const struct xa_load *l, unsigned n, uint32_t entry,
		   struct xa_plan *p)
{
	unsigned i;
	int text = -1;
	uint32_t win = 0;

	for (i = 0; i < n; i++) {
		uint32_t e = entry & ~1u;

		if (e - l[i].vaddr < l[i].filesz && e + 2 <= l[i].vaddr + l[i].filesz)
			text = (int)i;
	}
	if (text < 0 || l[text].filesz != l[text].memsz)
		return -1;
	p->text_link = l[text].vaddr;
	p->text_len = l[text].filesz;
	p->text_file = l[text].offset;
	p->entry_off = entry - l[text].vaddr;
	p->ndata = 0;
	for (i = 0; i < n; i++) {
		struct xa_range *d;

		if ((int)i == text || !l[i].memsz)
			continue;
		if (l[i].filesz > l[i].memsz || p->ndata >= XA_MAXRANGE)
			return -1;
		d = &p->data[p->ndata++];
		d->link = l[i].vaddr;
		d->mem = l[i].memsz;
		d->off = l[i].offset;
		d->len = l[i].filesz;
		d->win = win;
		win += l[i].memsz;
	}
	return 0;
}

/* Classify a relocated address: 0 = text, otherwise data range index + 1,
 * or -1 for anything outside every range. */
static int xa_class(const struct xa_plan *p, uint32_t v)
{
	unsigned i;

	if (v - p->text_link < p->text_len)
		return 0;
	for (i = 0; i < p->ndata; i++) {
		if (v - p->data[i].link < p->data[i].mem)
			return 1 + (int)i;
	}
	return -1;
}

/* Bias one relocated data word: a text target moves to the flash run, a
 * data target into the RAM window. Anything else aborts the install. */
static int xa_rebase(uint32_t *v, uint32_t type, const struct xa_plan *p,
		     uint32_t text_run, uint32_t win_base)
{
	uint32_t x = *v, win = 0;
	unsigned i;

	if (type != R_ARM_RELATIVE)
		return -1;
	if (x - p->text_link < p->text_len) {
		*v = text_run + (x - p->text_link);
		return 0;
	}
	for (i = 0; i < p->ndata; i++) {
		if (x - p->data[i].link < p->data[i].mem) {
			*v = win_base + win + (x - p->data[i].link);
			return 0;
		}
		win += p->data[i].mem;
	}
	return -1;
}

/* Program a run of pages from the file image, with no relocations. */
static int xa_raw(inoptr ino, uint32_t file, uint32_t len, uint32_t dst,
		  uint32_t ext, uint32_t from, uint32_t to)
{
	uint32_t i;

	for (i = from; i < to; i++) {
		uint32_t rem = len - i * XA_PAGE;
		uint8_t *pg = xa_slot(XA_ROFF + ext + dst + i * XA_PAGE);

		if (!pg)
			return -1;
		memset(pg, 0xff, XA_PAGE);
		if (xa_read(ino, file + i * XA_PAGE, pg,
			    rem > XA_PAGE ? XA_PAGE : rem))
			return -1;
	}
	return 0;
}

/* Program one piece (text or one data range) at extent offset dst. The
 * piece's relocations arrive in offset order, so pages between two
 * relocated pages are filled raw as the walk reaches them and each file
 * page is staged exactly once. */
static int xa_piece(inoptr ino, const struct xa_plan *p, unsigned cls,
		    uint32_t relf, uint32_t reln, uint32_t file, uint32_t len,
		    uint32_t dst, uint32_t ext, uint8_t *rels)
{
	uint32_t base = cls ? p->data[cls - 1].link : p->text_link;
	uint32_t filesz = cls ? p->data[cls - 1].len : p->text_len;
	uint32_t npages = (len + XA_PAGE - 1) / XA_PAGE;
	uint32_t run = text_run(ext);
	uint32_t r = 0, cur = 0, next = 0;
	uint8_t *pg = NULL;

	if (!len)
		return 0;		/* BSS only: zeroed at load */
	if (npages > 512)
		return -1;

	while (r < reln) {
		uint32_t n = reln - r, k;

		if (n > XA_PAGE / 8)
			n = XA_PAGE / 8;
		if (xa_read(ino, relf + r * 8, rels, n * 8))
			return -1;
		for (k = 0; k < n; k++) {
			Elf32_Rel rel;
			uint32_t a, page, off, w, v;
			int c;

			memcpy(&rel, rels + k * 8, sizeof(rel));
			a = rel.r_offset;
			c = xa_class(p, a);
			if (c < 0)
				return -1;	/* outside every range */
			if (c != (int)cls)
				continue;	/* another piece */
			if (a - base >= filesz)
				return -1;	/* only BSS backs it */
			page = (a - base) / XA_PAGE;
			if (pg && page < cur)
				return -1;	/* not in offset order */
			if (!pg || page != cur) {
				if (xa_raw(ino, file, len, dst, ext, next, page))
					return -1;
				cur = page;
				next = page + 1;
				pg = xa_slot(XA_ROFF + ext + dst + cur * XA_PAGE);
				if (!pg)
					return -1;
				memset(pg, 0xff, XA_PAGE);
				if (xa_read(ino, file + cur * XA_PAGE, pg,
					    min(len - cur * XA_PAGE, XA_PAGE)))
					return -1;
			}
			off = (a - base) & (XA_PAGE - 1);
			memcpy(&w, pg + off, sizeof(w));
			v = w;
			if (xa_rebase(&v, ELF32_R_TYPE(rel.r_info), p, run,
				      (uint32_t)PROGLOAD))
				return -1;
			memcpy(pg + off, &v, sizeof(v));
		}
		r += n;
	}
	if (xa_raw(ino, file, len, dst, ext, next, npages))
		return -1;
	return xa_flush();
}

/* Install the binary described by plan. Writes the header, the payload and
 * finally the valid marker; advances the append point past the extent even
 * on failure, so torn space is skipped but never reused. */
static int xa_install(inoptr ino, const struct xa_plan *p, uaddr_t himem,
		      uint32_t relf, uint32_t reln, uint8_t *rels,
		      struct exec_xip *image, int *index)
{
	struct xa_hdr h;
	uint32_t text_dst = (p->text_len + XA_PAGE - 1) & ~(XA_PAGE - 1);
	uint32_t total = XA_HDR + text_dst;
	uint32_t data_dst, data = 0, bss = 0, ext, i;

	for (i = 0; i < p->ndata; i++) {
		data += p->data[i].len;
		bss += p->data[i].mem - p->data[i].len;
		total += (p->data[i].len + XA_PAGE - 1) & ~(XA_PAGE - 1);
	}
	if (xa_fits(data, bss, himem))
		return 0;
	if (total > XA_LEN || ntab >= XA_NTABLE || bump + total > XA_LEN)
		return 0;
	if (!xa_quiet())		/* concurrency may have started */
		return 0;
	busy = 1;

	while (bump + total > erased) {
		if (!xa_blank(erased) && xa_erase_sector(XA_ROFF + erased))
			goto abort;
		erased += XA_SECTOR;
	}
	ext = bump;

	memset(&h, 0, sizeof(h));
	h.magic = XA_MAGIC;
	h.valid = 0xffffffffu;
	h.total = total;
	h.dev = ino->c_dev;
	h.num = ino->c_num;
	h.mtime = ino->c_node.i_mtime;
	h.size = ino->c_node.i_size;
	h.pool = (uint32_t)PROGBASE;
	h.entry_off = p->entry_off;
	h.text_len = p->text_len;
	h.nranges = p->ndata;
	data_dst = XA_HDR + text_dst;
	for (i = 0; i < p->ndata; i++) {
		h.data[i] = p->data[i];
		h.data[i].off = data_dst;
		data_dst += (p->data[i].len + XA_PAGE - 1) & ~(XA_PAGE - 1);
	}

	if (!xa_header(ext, &h) ||
	    xa_piece(ino, p, 0, relf, reln, p->text_file, p->text_len, XA_HDR,
		     ext, rels))
		goto abort;
	data_dst = XA_HDR + text_dst;
	for (i = 0; i < p->ndata; i++) {
		if (xa_piece(ino, p, 1 + i, relf, reln, p->data[i].off,
			     p->data[i].len, data_dst, ext, rels))
			goto abort;
		data_dst += (p->data[i].len + XA_PAGE - 1) & ~(XA_PAGE - 1);
	}
	h.valid = XA_VALID;
	if (!xa_header(ext, &h) || xa_flush())
		goto abort;

	tab[ntab] = h;
	tab_off[ntab] = ext;
	bump = ext + total;
	image->entry = text_base(ntab) + h.entry_off;
	image->data_size = data;
	image->bss_size = bss;
	*index = (int)ntab++;
	busy = 0;
	return 1;

abort:
	pnum = 0;			/* discard staged pages */
	bump = ext + total;
	busy = 0;
	return 0;
}

/* Fill image and index from a table entry (hit path). */
static int xa_use(int idx, uaddr_t himem, struct exec_xip *image, int *index)
{
	const struct xa_hdr *h = &tab[idx];
	uint32_t data = 0, bss = 0, i;

	for (i = 0; i < h->nranges; i++) {
		data += h->data[i].len;
		bss += h->data[i].mem - h->data[i].len;
	}
	if (xa_fits(data, bss, himem))
		return 0;
	image->entry = text_base(idx) + h->entry_off;
	image->data_size = data;
	image->bss_size = bss;
	*index = idx;
	return 1;
}

static uint32_t get32(const uint8_t *p)
{
	uint32_t v;

	memcpy(&v, p, sizeof(v));
	return v;
}

/* Inspect the extent at region[off]. *adv is the next offset to try, set in
 * every case so a torn header can never hide a later extent. Returns 1 and
 * fills h for a valid extent, -1 for a torn one, 0 otherwise. */
static int xa_probe(const uint8_t *r, uint32_t rlen, uint32_t off,
		    struct xa_hdr *h, uint32_t *adv)
{
	uint32_t total;

	*adv = XA_PAGE;
	if (off + XA_HDR > rlen || get32(r + off) != XA_MAGIC)
		return 0;
	total = get32(r + off + 8);
	if (total < XA_HDR || (total & (XA_PAGE - 1)) || off + total > rlen)
		return 0;
	*adv = total;
	if (get32(r + off + 4) != XA_VALID)
		return -1;
	memcpy(h, r + off, sizeof(*h));
	return h->nranges > XA_MAXRANGE ? 0 : 1;
}

/* Rebuild the RAM table by walking the region's extents. A torn extent
 * means a power cut mid-install; nothing can be running from flash yet,
 * so start over. */
static void xa_scan(void)
{
	const uint8_t *r = XA_MEM;
	uint32_t off = 0, end = 0;

	ntab = 0;
	while (off < XA_LEN) {
		struct xa_hdr h;
		uint32_t adv;
		int k = xa_probe(r, XA_LEN, off, &h, &adv);

		if (k == 1) {
			if (ntab < XA_NTABLE) {
				tab[ntab] = h;
				tab_off[ntab++] = off;
			}
			end = off + adv;
		} else if (k < 0) {
			rpipico_flash_erase(XA_ROFF, XA_LEN);
			ntab = 0;
			end = 0;
			break;
		}
		off += adv;
	}
	bump = end;
	erased = (end + XA_SECTOR - 1) & ~(XA_SECTOR - 1);
	ready = 1;
}

/* Linear lookup; the table is tiny and exec is not hot. */
static int xa_find(inoptr ino)
{
	unsigned i;

	for (i = 0; i < ntab; i++) {
		const struct xa_hdr *h = &tab[i];

		if (h->dev == ino->c_dev && h->num == ino->c_num &&
		    h->mtime == ino->c_node.i_mtime &&
		    h->size == ino->c_node.i_size &&
		    h->pool == (uint32_t)PROGBASE)
			return (int)i;
	}
	return -1;
}

int xip_auto_try(inoptr ino, const void *ehdr_v, uaddr_t himem,
		 struct exec_xip *image, int *index)
{
	const Elf32_Ehdr *e = (const Elf32_Ehdr *)ehdr_v;
	struct xa_parse *P;
	uint8_t *buf;
	uint32_t dyn = 0, dynsz = 0, relf = 0, reln = 0;
	unsigned n = 0, i;
	int idx, rel_ok = 0, sz_ok = 0;

	if (!ready)
		xa_scan();
	if (busy)
		return 0;
	idx = xa_find(ino);
	if (idx >= 0)
		return xa_use(idx, himem, image, index);
	if (!IS_ELF(*e) || e->e_type != ET_DYN || e->e_machine != EM_ARM)
		return 0;

	buf = tmpbuf();
	P = (struct xa_parse *)tmpbuf();
	if (e->e_phentsize != sizeof(Elf32_Phdr) || e->e_phnum > 8 ||
	    e->e_phnum * sizeof(Elf32_Phdr) > XA_PAGE ||
	    xa_read(ino, e->e_phoff, buf, e->e_phnum * sizeof(Elf32_Phdr)))
		goto out;
	for (i = 0; i < e->e_phnum; i++) {
		Elf32_Phdr ph;

		memcpy(&ph, buf + i * sizeof(ph), sizeof(ph));
		if (ph.p_type == PT_INTERP)
			goto out;
		if (ph.p_type == PT_DYNAMIC) {
			dyn = ph.p_offset;
			dynsz = ph.p_filesz;
		} else if (ph.p_type == PT_LOAD) {
			if (n >= 8)
				goto out;
			P->loads[n].vaddr = ph.p_vaddr;
			P->loads[n].filesz = ph.p_filesz;
			P->loads[n].memsz = ph.p_memsz;
			P->loads[n++].offset = ph.p_offset;
		}
	}
	if (!dynsz || dynsz > XA_PAGE ||
	    xa_plan(P->loads, n, e->e_entry, &P->plan) ||
	    P->plan.text_len < XA_MIN_TEXT)
		goto out;
	/* Installs need a quiet system. A forked child routinely reaches
	 * exec before its parent blocks in wait, so the caller may yield
	 * and retry once; nothing is held across the yield. */
	if (!xa_quiet()) {
		tmpfree(P);
		tmpfree(buf);
		return -1;
	}
	if (xa_read(ino, dyn, buf, dynsz))
		goto out;
	for (i = 0; (i + 1) * 8 <= dynsz; i++) {
		Elf32_Dyn d;

		memcpy(&d, buf + i * 8, sizeof(d));
		if (d.d_tag == DT_NULL)
			break;
		if (d.d_tag == DT_REL) {
			if (xa_map(P->loads, n, d.d_un.d_ptr, &relf))
				goto out;
			rel_ok = 1;
		} else if (d.d_tag == DT_RELSZ) {
			if (d.d_un.d_val % 8)
				goto out;
			reln = d.d_un.d_val / 8;
			sz_ok = 1;
		}
	}
	if (!rel_ok || !sz_ok)
		goto out;
	idx = xa_install(ino, &P->plan, himem, relf, reln, buf, image, index);
	tmpfree(P);
	tmpfree(buf);
	return idx ? 1 : 0;

out:
	tmpfree(P);
	tmpfree(buf);
	return 0;
}

/* Copy an installed image's data into the RAM window and zero its BSS. */
void xip_auto_load(int index)
{
	const struct xa_hdr *h = &tab[index];
	const uint8_t *b = XA_MEM + tab_off[index];
	uint32_t i;

	for (i = 0; i < h->nranges; i++) {
		memcpy((uint8_t *)PROGLOAD + h->data[i].win,
		       b + h->data[i].off, h->data[i].len);
		memset((uint8_t *)PROGLOAD + h->data[i].win + h->data[i].len, 0,
		       h->data[i].mem - h->data[i].len);
	}
	/* udata is copied on fork, rearranged on scheduling, and saved on
	 * swap. Ordinary exec resets this field. */
	udata.u_codebase = text_base((unsigned)index);
}

/* Validate a user pointer against the running process's XIP text. */
usize_t xip_auto_readable(uaddr_t addr, usize_t size)
{
	unsigned i;
	uint32_t base;

	for (i = 0; i < ntab; i++) {
		if (text_base(i) == udata.u_codebase)
			break;
	}
	if (i >= ntab)
		return 0;
	base = text_base(i);
	if (addr - base >= tab[i].text_len)
		return 0;
	return min(size, base + tab[i].text_len - addr);
}
