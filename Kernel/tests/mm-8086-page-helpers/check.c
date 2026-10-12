#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t page_t;
#define PAGESIZE 4096
#define PAGESHIFT 12
#define PAGES_64K 16
#define NUM_PAGES 160
static uint8_t pagemap[NUM_PAGES];
static uint16_t free_pages,total_pages;
static unsigned char ram[NUM_PAGES*PAGESIZE],expected[NUM_PAGES*PAGESIZE];
static void panic(const char *s) { abort(); }
static void do_copy_page(uint16_t to,uint16_t from) { memcpy(ram+(unsigned)to*16,ram+(unsigned)from*16,PAGESIZE); }
static void do_zero_page(uint16_t to) { memset(ram+(unsigned)to*16,0,PAGESIZE); }
/* FUNCTION mm/bank8086.c page_to_segment */
/* FUNCTION mm/bank8086.c need_zeros */
/* FUNCTION mm/bank8086.c skip_used */
/* FUNCTION mm/bank8086.c alloc_data */
/* FUNCTION mm/bank8086.c alloc_code */
/* FUNCTION mm/bank8086.c copy_pages */
/* FUNCTION mm/bank8086.c zero_pages */
/* FUNCTION mm/bank8086.c pagemap_init_range */
int main(void) {
 pagemap_init_range(65,640);assert(pagemap[16]==254 && pagemap[17]==0);
 assert(total_pages==143 && free_pages==143);
 assert(alloc_code(2)==17 && alloc_data(2,2)==17);
 memset(pagemap,254,sizeof(pagemap));assert(alloc_code(1)==0 && alloc_data(1,1)==0);
 pagemap[159]=0;assert(alloc_code(2)==0 && alloc_data(1,1)==0);
 assert(need_zeros(pagemap+159,2)==1);
 pagemap_init_range(64,512);assert(pagemap[127]==0 && pagemap[128]==254 && total_pages==112);
 for(unsigned i=0;i<sizeof(ram);i++)ram[i]=(i/PAGESIZE)*17;
 memcpy(expected,ram,sizeof(ram));memmove(expected+2*PAGESIZE,expected+PAGESIZE,3*PAGESIZE);
 copy_pages(2,1,3);assert(memcmp(expected,ram,sizeof(ram))==0);
 memmove(expected+PAGESIZE,expected+2*PAGESIZE,3*PAGESIZE);copy_pages(1,2,3);
 assert(memcmp(expected,ram,sizeof(ram))==0);
 zero_pages(1,3);for(unsigned i=PAGESIZE;i<4*PAGESIZE;i++)assert(ram[i]==0);
 return 0;
}
