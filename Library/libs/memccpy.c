/*
 *	memccpy.c: copy memory until match or end marker
 */

#include <string.h>

void *memccpy(void *d, const void *s, int c, register size_t n)
{
  register unsigned char *s1 = d;
  register const unsigned char *s2 = s;
  while (n) {
    n--;
    if ((*s1++ = *s2++) == (unsigned char)c)
      return s1;
  }
  return NULL;
}
