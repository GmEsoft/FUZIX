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
#include <sys/wait.h>
static int interrupt_once;
static pid_t test_waitpid(pid_t pid,int *status,int options){if(interrupt_once){interrupt_once=0;errno=EINTR;return -1;}return waitpid(pid,status,options);}
#define waitpid test_waitpid
#define popen fuzix_popen
#define pclose fuzix_pclose
/* SOURCE libs/popen.c */
int main(void){pid_t unrelated=fork();assert(unrelated>=0);if(!unrelated)_exit(55);FILE *p=fuzix_popen("exit 7","r");assert(p);interrupt_once=1;int status=fuzix_pclose(p);assert(WIFEXITED(status)&&WEXITSTATUS(status)==7);int other;assert(test_waitpid(unrelated,&other,0)==unrelated&&WEXITSTATUS(other)==55);FILE *a=fuzix_popen("printf hello","r"),*b=fuzix_popen("exit 9","r");assert(a&&b);char text[8];assert(fgets(text,sizeof text,a)&&!strcmp(text,"hello"));assert(fuzix_pclose(a)==0);status=fuzix_pclose(b);assert(WIFEXITED(status)&&WEXITSTATUS(status)==9);FILE *w=fuzix_popen("cat >/dev/null","w");assert(w);assert(fputs("hello",w)>=0&&fuzix_pclose(w)==0);FILE *plain=tmpfile();assert(plain);assert(fuzix_pclose(plain)==-1&&errno==EINVAL);fclose(plain);assert(!fuzix_popen("true","bad")&&errno==EINVAL);return 0;}
