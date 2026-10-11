/* Exercise the production swapper with a byte-addressed disk and udata alias. */
#include <kernel.h>
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAPS 32
static uint8_t disk[MAPS * TEST_PROCMEM];
static bool available[MAPS];
static unsigned writes, reads, flushes, trims, fail_write, fail_read;
static int short_io, fail_flush, catch_panic;
static jmp_buf failure;
struct p_tab ptab[PTABSIZE];
uint16_t swap_dev = 0;
void kprintf(const char *fmt, ...) { (void)fmt; }
void panic(const char *message)
{
    if (catch_panic) longjmp(failure, 1);
    fprintf(stderr, "panic: %s\n", message);
    abort();
}
int16_t swapmap_alloc(void)
{
    for (unsigned i=0; i<MAPS; i++)
        if (available[i]) { available[i]=false; return i; }
    return -1;
}
void swapmap_add(uint_fast8_t map)
{
    assert(map < MAPS && !available[map]);
    available[map]=true;
}
static unsigned maps_left(void)
{
    unsigned n=0;
    for (unsigned i=0; i<MAPS; i++) n+=available[i];
    return n;
}
ptptr swapneeded(ptptr p, int notself)
{
    for (unsigned i=0; i<PTABSIZE; i++)
        if (&ptab[i] != p && (!notself || &ptab[i] != udata.u_ptab) && ptab[i].p_page)
            return swapout(&ptab[i]) ? NULL : &ptab[i];
    return NULL;
}
int d_ioctl(uint16_t dev, unsigned cmd, void *data)
{
    (void)dev;
    assert(cmd==HDIO_TRIM_KERNEL);
    blkno_t block=*(blkno_t *)data;
    assert(!available[block / SWAP_SIZE]);
    assert(udata.u_ptab && udata.u_ptab->p_page);
    memset(disk + block * 512,0,512);
    udata.u_error=EIO;
    trims++;
    return 0;
}
int d_flush(uint16_t dev)
{
    (void)dev;
    flushes++;
    return fail_flush ? -1 : 0;
}
int swapwrite(uint16_t dev, uint16_t block, usize_t len, uaddr_t buf, uint16_t page)
{
    (void)dev; (void)page;
    assert((size_t)block * 512 + len <= sizeof(disk));
    if (++writes == fail_write) return short_io ? len - 512 : -1;
    memcpy(disk + block * 512, (void *)buf, len);
    return len;
}
int swapread(uint16_t dev, uint16_t block, usize_t len, uaddr_t buf, uint16_t page)
{
    (void)dev; (void)page;
    assert((size_t)block * 512 + len <= sizeof(disk));
    /* Model generic swapread's writes through the fixed udata alias. If a
     * user block is read into physical block zero early, this corrupts it. */
    udata.u_isp = block;
    if (++reads == fail_read) return short_io ? len - 512 : -1;
    memcpy((void *)buf, disk + block * 512, len);
    return len;
}
void swap_blocks(void *a, void *b, size_t len)
{
    uint8_t *x=a, *y=b;
    while (len--) { uint8_t t=*x; *x++=*y; *y++=t; }
}
static void reset(unsigned bytes)
{
    memset(ptab,0,sizeof(ptab));
    memset(progbase,0,USERMEM);
    memset(available,1,sizeof(available));
    memset(disk,0,sizeof(disk));
    writes=reads=flushes=trims=fail_write=fail_read=0;
    fail_flush=short_io=catch_panic=0;
    pagemap_init();
    ptab[0].p_top=PROGBASE+bytes;
    assert(pagemap_alloc(&ptab[0])==0);
    udata.u_ptab=&ptab[0];
    udata.u_top=ptab[0].p_top;
    udata.u_break=ptab[0].p_top;
    memset(progbase+UDATA_SIZE,0xa5,bytes-UDATA_SIZE);
}
static void reuse(void)
{
    reset(8192);
    for (unsigned i=0; i<MAPS*3; i++) {
        assert(swapout(&ptab[0])==0);
        assert(maps_left()==MAPS-1);
        contextswitch(&ptab[0]);
        assert(maps_left()==MAPS && trims==(i+1)*SWAP_SIZE);
        assert(udata.u_error==0);
        assert(udata.u_ptab==&ptab[0]);
        for (unsigned j=UDATA_SIZE; j<8192; j++) assert(progbase[j]==0xa5);
    }
}
static void write_failure(unsigned at, int partial, int flush)
{
    reset(16384);
    fail_write=at; short_io=partial; fail_flush=flush;
    assert(swapout(&ptab[0])==EIO);
    assert(ptab[0].p_page && maps_left()==MAPS);
    assert(pagemap_mem_used()==16);
    for (unsigned j=UDATA_SIZE; j<16384; j++) assert(progbase[j]==0xa5);
    fail_write=fail_flush=0;
    assert(swapout(&ptab[0])==0);
    assert(!ptab[0].p_page && pagemap_mem_used()==0 && flushes);
}
static void read_failure(unsigned at, int partial)
{
    reset(16384);
    assert(swapout(&ptab[0])==0);
    uint16_t map=ptab[0].p_page2;
    fail_read=at; short_io=partial; catch_panic=1;
    if (!setjmp(failure)) { contextswitch(&ptab[0]); abort(); }
    catch_panic=0;
    assert(!ptab[0].p_page && ptab[0].p_page2==map);
    assert(maps_left()==MAPS-1 && pagemap_mem_used()==0 && !trims);
    fail_read=0;
    contextswitch(&ptab[0]);
    assert(maps_left()==MAPS && udata.u_ptab==&ptab[0]);
    for (unsigned j=UDATA_SIZE; j<16384; j++) assert(progbase[j]==0xa5);
}
static void exhausted(void)
{
    reset(4096);
    memset(available,0,sizeof(available));
    assert(swapout(&ptab[0])==ENOMEM);
    assert(ptab[0].p_page && pagemap_mem_used()==4 && !writes);
}
static void allocation_failure(void)
{
    reset(TEST_PROCMEM);
    ptab[1].p_top=PROGBASE+TEST_PROCMEM;
    ptab[1].p_page2=0;
    memset(available,0,sizeof(available));
    catch_panic=1;
    if (!setjmp(failure)) { contextswitch(&ptab[1]); abort(); }
    catch_panic=0;
    assert(ptab[0].p_page && !ptab[1].p_page);
    assert(pagemap_mem_used()==TEST_PROCMEM/1024);
    assert(!reads && !trims);
    for (unsigned j=UDATA_SIZE; j<TEST_PROCMEM; j++) assert(progbase[j]==0xa5);
}
static void large(void)
{
    unsigned bytes=TEST_PROCMEM;
    reset(bytes);
    assert(swapout(&ptab[0])==0);
    ptab[1].p_top=PROGBASE+bytes;
    assert(pagemap_alloc(&ptab[1])==0);
    udata.u_ptab=&ptab[1];
    udata.u_top=ptab[1].p_top;
    udata.u_break=ptab[1].p_top;
    memset(progbase+UDATA_SIZE,0x5a,bytes-UDATA_SIZE);
    for (unsigned i=0; i<4; i++) {
        unsigned slot=i%2;
        contextswitch(&ptab[slot]);
        assert(udata.u_ptab==&ptab[slot]);
        assert(maps_left()==MAPS-1);
        for (unsigned j=UDATA_SIZE; j<bytes; j++) assert(progbase[j]==(slot ? 0x5a : 0xa5));
    }
}
int main(int argc, char **argv)
{
    assert(argc==2);
    if (!strcmp(argv[1],"reuse")) reuse();
    else if (!strcmp(argv[1],"write-error")) write_failure(1,0,0);
    else if (!strcmp(argv[1],"partial-write")) write_failure(3,1,0);
    else if (!strcmp(argv[1],"flush-error")) write_failure(0,0,1);
    else if (!strcmp(argv[1],"read-error")) read_failure(1,0);
    else if (!strcmp(argv[1],"partial-read")) read_failure(3,1);
    else if (!strcmp(argv[1],"exhausted")) exhausted();
    else if (!strcmp(argv[1],"large")) large();
    else if (!strcmp(argv[1],"allocation-failure")) allocation_failure();
    else abort();
    puts(argv[1]);
}
