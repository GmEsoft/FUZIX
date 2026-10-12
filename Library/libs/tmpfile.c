#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

FILE *tmpfile(void)
{
  char name[] = "/tmp/tmpfileXXXXXX";
  FILE *fp;
  int saved_errno;
  int fd = mkstemp(name);
  if (fd == -1)
    return NULL;
  if (unlink(name) == -1) {
    saved_errno = errno;
    close(fd);
    errno = saved_errno;
    return NULL;
  }
  fp = fdopen(fd, "r+");
  if (fp == NULL) {
    saved_errno = errno;
    close(fd);
    errno = saved_errno;
  }
  return fp;
}
