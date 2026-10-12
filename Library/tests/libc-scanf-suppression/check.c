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
#include <stdarg.h>
#include <ctype.h>
/* STDIO */
static const char *input;static size_t pos;
static int test_getc(FILE *fp){return input[pos]?(unsigned char)input[pos++]:EOF;}
static int test_ungetc(int c,FILE *fp){if(c!=EOF&&pos)pos--;return c;}
#define fgetc test_getc
#define getc test_getc
#define ungetc test_ungetc
#define vfscanf fuzix_vfscanf
/* SOURCE libs/vfscanf.c */
static int scan(const char *s,const char *fmt,...){FILE f={0};va_list ap;input=s;pos=0;va_start(ap,fmt);int r=fuzix_vfscanf(&f,fmt,ap);va_end(ap);return r;}
int main(void){short n=0;assert(scan("skip 42","%*s %hd",&n)==1&&n==42);n=0;assert(scan("x37","%*c%hd",&n)==1&&n==37);n=0;assert(scan("abc 19","%*[a-z] %hd",&n)==1&&n==19);return 0;}
