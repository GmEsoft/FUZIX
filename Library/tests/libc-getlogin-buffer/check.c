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
#define UT_LINESIZE 12
#define UT_NAMESIZE 8
#define USER_PROCESS 7
#define L_cuserid 9
struct utmp {char ut_line[UT_LINESIZE];int ut_type;char ut_user[UT_NAMESIZE];};
static struct utmp user;static int test_open(const char *p,int flags){return 7;}static int test_close(int fd){return 0;}
static int test_ttyname(int fd,char *p,size_t n){const char *s="/dev/tty1234";assert(n>=strlen(s)+1);strcpy(p,s);return 0;}
static void setutent(void){}static void endutent(void){}static struct utmp *getutline(const struct utmp *u){assert(!strcmp(u->ut_line,"tty1234"));return &user;}
static size_t test_strlcpy(char *d,const char *s,size_t n){size_t l=strlen(s);if(n){size_t c=l<n?l:n-1;memcpy(d,s,c);d[c]=0;}return l;}
#define strlcpy test_strlcpy
#define open test_open
#define close test_close
#define ttyname_r test_ttyname
#define getlogin fuzix_getlogin
#define getlogin_r fuzix_getlogin_r
int fuzix_getlogin_r(char *,size_t);
/* SOURCE libs/getlogin.c */
int main(void){char *b=malloc(9);memcpy(user.ut_user,"abcdefgh",8);assert(fuzix_getlogin_r(b,9)==0&&!strcmp(b,"abcdefgh"));assert(fuzix_getlogin_r(b,8)==-1&&errno==ERANGE);assert(fuzix_getlogin_r(NULL,0)==-1&&errno==ERANGE);free(b);return 0;}
