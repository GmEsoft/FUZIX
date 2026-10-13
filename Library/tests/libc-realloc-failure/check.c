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
static int calls; static void *fail_malloc(size_t n){calls++; return NULL;}
#define malloc fail_malloc
#define realloc fuzix_realloc
/* SOURCE libs/realloc.c */
#undef malloc
int main(void) { struct memh *h=malloc(2*sizeof *h); h->size=2; unsigned char *p=(void *)(h+1); p[0]=42; assert(fuzix_realloc(p,1000)==NULL && p[0]==42 && calls==1); calls=0; assert(fuzix_realloc(p,SIZE_MAX)==NULL && calls==0 && p[0]==42); free(h); return 0; }
