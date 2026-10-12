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
static int unlinked, lastfd, fail_open, fail_unlink;
static int test_mkstemp(char *p){assert(strstr(p,"XXXXXX")); memcpy(p+strlen(p)-6,"123456",6);return lastfd=open("/dev/null",O_RDWR);}
static int test_unlink(const char *p){unlinked++;if(fail_unlink){errno=EACCES;return -1;}return 0;}
static FILE *test_fdopen(int fd,const char *mode){if(fail_open){errno=ENOMEM;return NULL;}return fdopen(fd,mode);}
#define mkstemp test_mkstemp
#define unlink test_unlink
#define fdopen test_fdopen
#define tmpfile fuzix_tmpfile
/* SOURCE libs/tmpfile.c */
int main(void) { FILE *a=fuzix_tmpfile(),*b=fuzix_tmpfile();assert(a&&b&&unlinked==2);fclose(a);fclose(b);fail_open=1;assert(!fuzix_tmpfile()&&errno==ENOMEM);assert(fcntl(lastfd,F_GETFD)==-1&&errno==EBADF);fail_open=0;fail_unlink=1;assert(!fuzix_tmpfile()&&errno==EACCES);assert(fcntl(lastfd,F_GETFD)==-1&&errno==EBADF);return 0; }
