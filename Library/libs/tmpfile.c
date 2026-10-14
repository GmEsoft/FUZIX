#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

FILE *tmpfile(void)
{
  char name[19];
  FILE *fp;
  int saved_errno;
  int fd;

  memcpy(name, "/tmp/tmpfileXXXXXX", sizeof(name));
  fd = mkstemp(name);
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
