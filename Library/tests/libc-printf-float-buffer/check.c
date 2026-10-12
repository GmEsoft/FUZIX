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
#include <ctype.h>
#include <stdarg.h>
/* STDIO */
#define BUILD_LIBM
#define ecvt fuzix_ecvt
#define fcvt fuzix_fcvt
/* SOURCE libs/ecvt.c */
/* SOURCE libs/_fnum.c */
/* SOURCE libs/ltostr.c */
static int test_fputc(int c,FILE *fp){assert(fp->bufpos<fp->bufend);*fp->bufpos++=c;return (unsigned char)c;}
static int test_fflush(FILE *fp){return 0;}
#define fputc test_fputc
#define fflush test_fflush
/* FUNCTION libs/vfprintf.c prtfld */
/* FUNCTION libs/vfprintf.c _vfnprintf */
static int format(char *b,size_t size,const char *fmt,...){FILE f={0};f.mode=__MODE_WRITE;f.bufpos=f.bufstart=(unsigned char *)b;f.bufend=f.bufstart+size;va_list ap;va_start(ap,fmt);int n=_vfnprintf(&f,size-1,fmt,ap);va_end(ap);*f.bufpos=0;return n;}
int main(void){char b[512],small[4];double cases[]={1e100,DBL_MAX,-DBL_MAX,1e-300};for(size_t i=0;i<sizeof cases/sizeof cases[0];i++){int n=format(b,sizeof b,"%f",cases[i]);assert(n>64&&n<512&&(size_t)n==strlen(b));int m=format(small,sizeof small,"%f",cases[i]);assert(m==n&&strlen(small)==3);}return 0;}
