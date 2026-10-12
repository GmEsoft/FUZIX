#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
struct memh { struct memh *next; size_t size; };
#define MH(p) (((struct memh *)(p))-1)
static void *test_malloc(size_t n){return calloc(1,n);} static void test_free(void *p){free(MH(p));}
#define malloc test_malloc
#define free test_free
#define realloc fuzix_realloc
/* SOURCE libs/realloc.c */
#undef malloc
#undef free
int main(void) { struct memh *h=malloc(3*sizeof *h); h->size=3; unsigned char *p=(void *)(h+1); size_t len=2*sizeof *h; memset(p,42,len); unsigned char *q=fuzix_realloc(p,8*sizeof *h); assert(q && q!=p); for(size_t i=0;i<len;i++)assert(q[i]==42); free(q); return 0; }
