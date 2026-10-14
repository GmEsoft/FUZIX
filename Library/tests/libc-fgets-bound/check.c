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
#include <fcntl.h>
/* STDIO */
static const char *input; static size_t pos;
static int test_getc(FILE *f){return input[pos]?(unsigned char)input[pos++]:EOF;}
#define getc test_getc
#define fgets fuzix_fgets
/* SOURCE libs/fgets.c */
int main(void) { FILE f={0}; char *b=malloc(8); input="abcdefghij\n"; pos=0; assert(fuzix_fgets(b,8,&f)==b); assert(!strcmp(b,"abcdefg") && pos==7); assert(fuzix_fgets(NULL,0,&f)==NULL && pos==7); assert(fuzix_fgets(b,1,&f)==b && !*b && pos==7); input="x\ny";pos=0;assert(fuzix_fgets(b,8,&f)==b && !strcmp(b,"x\n")); input="";pos=0;assert(!fuzix_fgets(b,8,&f));free(b);return 0; }
