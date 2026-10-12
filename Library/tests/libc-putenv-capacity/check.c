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
static char *initial[]={NULL};static char **test_environ=initial;
#define environ test_environ
#define putenv fuzix_putenv
#define setenv fuzix_setenv
#define unsetenv fuzix_unsetenv
/* SOURCE libs/putenv.c */
/* SOURCE libs/setenv.c */
int main(void){char a[]="A=one",c[]="C=three",d[]="D=four";assert(fuzix_putenv(a)==0);assert(fuzix_setenv("B","two",1)==0);assert(fuzix_putenv(c)==0);assert(fuzix_putenv(d)==0);assert(!strcmp(test_environ[0],"A=one")&&!strcmp(test_environ[1],"B=two")&&!strcmp(test_environ[2],"C=three")&&!strcmp(test_environ[3],"D=four")&&!test_environ[4]);return 0;}
