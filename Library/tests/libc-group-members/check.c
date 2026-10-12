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
#include <grp.h>
#define GR_MAX_LINE_LEN 186
#define GR_MAX_MEMBERS 16
static const char *input;static size_t pos;
static ssize_t test_read(int fd,void *p,size_t n){size_t left=strlen(input)-pos;if(n>left)n=left;memcpy(p,input+pos,n);pos+=n;return n;}
static off_t test_lseek(int fd,off_t off,int whence){assert(whence==SEEK_CUR);pos+=off;return pos;}
#define read test_read
#define lseek test_lseek
/* SOURCE libs/__getgrent.c */
int main(void){char line[128]="g:x:1:";for(int i=0;i<16;i++)strcat(line,i?",a":"a");strcat(line,"\n");input=line;pos=0;struct group *g=__getgrent(7);assert(g);for(int i=0;i<16;i++)assert(g->gr_mem[i]&&!strcmp(g->gr_mem[i],"a"));assert(g->gr_mem[16]==NULL);strcpy(line+strlen(line)-1,",a\n");input=line;pos=0;errno=0;assert(__getgrent(7)==NULL&&errno==ERANGE);return 0;}
