#include <kernel.h>
#include <blkdev.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/dhara/map.h"
#include "../../globals.h"
#define NBYTES (PICO_FLASH_SIZE_BYTES-FLASH_OFFSET)
static uint8_t storage[NBYTES];
static blkdev_t device;
static bool fail_program;
struct u_data udata;
struct blkparam blk_op;
void kprintf(const char *fmt, ...) { (void)fmt; }
blkdev_t *blkdev_alloc(void) { memset(&device,0,sizeof(device)); return &device; }
void blkdev_scan(blkdev_t *d,uint_fast8_t flags) { assert(d==&device && !flags && d->drive_lba_count); }
int dhara_nand_erase(const struct dhara_nand *n,dhara_block_t b,dhara_error_t *err)
{
    assert(b<n->num_blocks);
    if (fail_program) { dhara_set_error(err,DHARA_E_BAD_BLOCK); return -1; }
    dhara_set_error(err,DHARA_E_NONE);
    memset(storage+b*4096,0xff,4096);
    return 0;
}
int dhara_nand_prog(const struct dhara_nand *n,dhara_page_t p,const uint8_t *data,dhara_error_t *err)
{
    assert(p<(n->num_blocks<<n->log2_ppb));
    if (fail_program) { dhara_set_error(err,DHARA_E_BAD_BLOCK); return -1; }
    dhara_set_error(err,DHARA_E_NONE);
    for (unsigned i=0;i<512;i++) { assert((storage[p*512+i]&data[i])==data[i]); storage[p*512+i]&=data[i]; }
    return 0;
}
int dhara_nand_read(const struct dhara_nand *n,dhara_page_t p,size_t off,size_t len,uint8_t *data,dhara_error_t *err)
{
    dhara_set_error(err,DHARA_E_NONE);
    assert(p<(n->num_blocks<<n->log2_ppb) && off+len<=512);
    memcpy(data,storage+p*512+off,len);
    return 0;
}
static void write_sector(unsigned sector,uint8_t value)
{
    static uint8_t data[512];
    memset(data,value,sizeof(data));
    blk_op.lba=sector; blk_op.addr=data; blk_op.is_read=false;
    assert(device.transfer && device.transfer());
}
static void check_sector(unsigned sector,uint8_t value)
{
    uint8_t data[512];
    blk_op.lba=sector; blk_op.addr=data; blk_op.is_read=true;
    assert(device.transfer());
    for (unsigned i=0;i<sizeof(data);i++) assert(data[i]==value);
}
int main(int argc,char **argv)
{
    assert(argc==2);
    memset(storage,0xff,sizeof(storage));
    flash_dev_init();
    assert(device.flush);
    if (!strcmp(argv[1],"restart")) {
        write_sector(0,0x11); assert(!device.flush());
        write_sector(0,0x22); write_sector(1,0x33); /* completed writes checkpoint themselves */
        flash_dev_init(); check_sector(0,0x22); check_sector(1,0x33);
        assert(!device.flush()); /* clean journal sync */
    } else if (!strcmp(argv[1],"trim")) {
        write_sector(0,0x11); write_sector(1,0x22); assert(!device.flush());
        assert(device.trim); blk_op.lba=0; assert(!device.trim()); assert(!device.flush());
        flash_dev_init(); check_sector(0,0xff); check_sector(1,0x22);
    } else if (!strcmp(argv[1],"failure")) {
        write_sector(0,0x11); assert(!device.flush());
        /* A trim dirties the journal without checkpointing on the write path. */
        blk_op.lba=0; assert(device.trim && !device.trim()); fail_program=true;
        assert(device.flush()==-1 && udata.u_error==EIO);
        fail_program=false;
        flash_dev_init(); check_sector(0,0x11);
    } else abort();
    puts(argv[1]);
}
