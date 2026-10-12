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
#include <pwd.h>
#include <grp.h>
#define GR_MAX_LINE_LEN 186
#define GR_MAX_MEMBERS 16
static const char *input;static size_t length,pos;
static ssize_t test_read(int fd,void *p,size_t n){size_t left=length-pos;if(n>left)n=left;memcpy(p,input+pos,n);pos+=n;return n;}
static off_t test_lseek(int fd,off_t off,int whence){assert(whence==SEEK_CUR);pos+=off;return pos;}
#define read test_read
#define lseek test_lseek
/* SOURCE libs/__getpwent.c */
/* SOURCE libs/__getgrent.c */
int main(void){char data[512];memset(data,'x',256);strcpy(data+256,"\nu:x:1:2:n:/home:/bin/sh\n");input=data;length=strlen(data);pos=0;struct passwd *p=__getpwent(7);assert(p&&!strcmp(p->pw_name,"u")&&p->pw_uid==1);memset(data,'x',186);strcpy(data+186,"\ng:x:1:me\n");length=strlen(data);pos=0;struct group *g=__getgrent(7);assert(g&&!strcmp(g->gr_name,"g")&&!strcmp(g->gr_mem[0],"me"));input="unfinished";length=strlen(input);pos=0;assert(!__getpwent(7));pos=0;assert(!__getgrent(7));return 0;}
