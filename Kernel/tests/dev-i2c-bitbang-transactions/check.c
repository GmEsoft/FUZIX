#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#define used(x) ((void)(x))
#define __packed __attribute__((packed))
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define BLKSHIFT 9
#define BLKSIZE 512
#define MAX_BLKDEV 2
#define kputs(x) ((void)(x))
#define kprintf(...) ((void)0)
static unsigned printed;
static void kputchar(int c) { (void)c; printed++; }
typedef uintptr_t uarg_t;
typedef uint16_t blkno_t;
struct { int u_error; uint16_t u_nblock,u_block,u_page; uint8_t *u_dptr,*u_base; unsigned u_count; } udata;
static int d_blkoff(int n) { (void)n; return 0; }

#define I2C_SCL 1
#define I2C_SDA 2
static unsigned pins=3,clocks,bitval,claims,releases,samplepos,samplecount;static bool receiving;
static uint8_t samples[32];
static void i2c_set(unsigned b,unsigned v){(void)b;if(v==I2C_SCL && !(pins&1)){clocks++;if(clocks<=8)bitval=(bitval<<1)|!!(pins&2);if(clocks==9)assert(pins&2);if(receiving&&clocks==18)assert(!(pins&2));if(receiving&&clocks==27)assert(pins&2);}pins|=v;}
static void i2c_clear(unsigned b,unsigned v){(void)b;pins&=~v;}
static unsigned i2c_scl(unsigned b){(void)b;return 1;}
static unsigned i2c_sda(unsigned b){(void)b;assert(samplepos<samplecount);return samples[samplepos++];}
static int i2c_claim_bus(unsigned b){(void)b;claims++;return 0;}
static void i2c_release_bus(unsigned b){(void)b;releases++;}
/* FUNCTION dev/i2c_bitbang.c i2c_clock_up */
/* FUNCTION dev/i2c_bitbang.c i2c_start */
/* FUNCTION dev/i2c_bitbang.c i2c_stop */
/* FUNCTION dev/i2c_bitbang.c i2c_read */
/* FUNCTION dev/i2c_bitbang.c i2c_write */
/* FUNCTION dev/i2c_bitbang.c i2c_bb_receive */
/* FUNCTION dev/i2c_bitbang.c i2c_bb_send */
static void reset(void){pins=3;clocks=bitval=samplepos=0;receiving=false;memset(samples,0,sizeof(samples));}
int main(void){reset();samplecount=1;assert(i2c_bb_send(0,0xa0,NULL,0)==0&&bitval==0xa0&&releases==claims);reset();samples[0]=1;samplecount=1;assert(i2c_bb_receive(0,0xa1,NULL,0)==-ENODEV&&claims==releases);reset();receiving=true;samplecount=17;for(unsigned i=0;i<8;i++){samples[1+i]=(0x96>>(7-i))&1;samples[9+i]=(0x63>>(7-i))&1;}uint8_t data[2]={0};assert(i2c_bb_receive(0,0xa1,data,2)==0&&data[0]==0x96&&data[1]==0x63&&claims==releases&&samplepos==17);reset();samplecount=3;assert(i2c_bb_send(0,0xa0,data,2)==0&&samplepos==3&&claims==releases);reset();samplecount=2;samples[1]=1;assert(i2c_bb_send(0,0xa0,data,2)==-EIO&&claims==releases);return 0;}
