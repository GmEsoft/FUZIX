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
#define GR_MAX_GROUPS 32
static int row,calls;static struct group group;static char *members[]={"user","user",NULL};
static struct group *test_getgrent(int fd){if(row==40)return NULL;group.gr_gid=100+row++;group.gr_mem=members;return &group;}
static int test_open(const char *p,int flags){return 7;}static int test_close(int fd){return 0;}
static int test_setgroups(size_t n,const gid_t *g){calls++;assert(n==32&&g[0]==7);for(size_t i=1;i<n;i++)assert(g[i]==99+i);return 0;}
#define open test_open
#define close test_close
#define __getgrent test_getgrent
#define setgroups test_setgroups
#define initgroups fuzix_initgroups
/* SOURCE libs/initgroups.c */
int main(void){assert(fuzix_initgroups("user",7)==0&&calls==1&&row==31);return 0;}
