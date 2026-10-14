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
static FILE *__IO_list; static int closed;
static int test_fflush(FILE *p){return 0;} static int test_close(int fd){closed++;return 0;}
#define fflush test_fflush
#define close test_close
#define fclose fuzix_fclose
/* SOURCE libs/fclose.c */
int main(void) { FILE *a=calloc(1,sizeof *a),*b=calloc(1,sizeof *b),*c=calloc(1,sizeof *c); a->mode=b->mode=c->mode=__MODE_FREEFIL; c->next=b;b->next=a;__IO_list=c; assert(fuzix_fclose(b)==0);assert(__IO_list==c && c->next==a); assert(fuzix_fclose(c)==0 && __IO_list==a);assert(fuzix_fclose(a)==0 && __IO_list==NULL && closed==3);return 0; }
