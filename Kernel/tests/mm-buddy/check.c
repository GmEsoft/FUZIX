#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define CONFIG_BUDDY_MALLOC
#define BUDDY_SIZE 16
#define BUDDY_NODESHIFT 5
static uint8_t pool[BUDDY_SIZE<<BUDDY_NODESHIFT];
static uint8_t *buddy_ram=pool;
static void panic(const char *s) { abort(); }
/* SOURCE mm/buddy.c */
int main(void) {
 buddy_init(BUDDY_SIZE,2*BUDDY_SIZE);
 assert(kmemavail()==sizeof(pool));
 uint8_t *a=kmalloc(1,1);uint8_t *b=kmalloc(64,2);
 assert(a==pool && b==pool+64 && kmemused()==96);
 assert(kmalloc(0,1)==NULL && kmalloc(sizeof(pool)+1,1)==NULL);
 assert(kmalloc(SIZE_MAX,1)==NULL && kmalloc(1,255)==NULL);
 kfree_s(a,1);kfree_s(b,64);assert(kmemused()==0 && buddy[1]==NODE_FREE);
 a=kmalloc(sizeof(pool),3);assert(a==pool && kmemavail()==0);
 assert(kmalloc(1,1)==NULL);kfree_s(a,sizeof(pool));kfree_s(NULL,1);
 buddy_init(BUDDY_SIZE+1,2*BUDDY_SIZE-1);
 assert(kmemavail()==sizeof(pool)-64 && kmalloc(sizeof(pool),1)==NULL);
 uint8_t *blocks[BUDDY_SIZE-2];
 for(unsigned i=0;i<BUDDY_SIZE-2;i++){blocks[i]=kmalloc(32,1);assert(blocks[i]>=pool+32 && blocks[i]<pool+sizeof(pool)-32);}
 assert(kmalloc(32,1)==NULL);
 for(unsigned i=0;i<BUDDY_SIZE-2;i++)kfree_s(blocks[i],32);
 assert(kmemavail()==sizeof(pool)-64);return 0;
}
