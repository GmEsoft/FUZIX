/* Another BSDism */

#include <string.h>
#include <ctype.h>

char *strcasestr(register const char *haystack, register const char *needle)
{
  size_t s = strlen(needle);

  while (*haystack) {
      if (strncasecmp(needle, haystack, s) == 0)
        return (char *)haystack;
    haystack++;
  }
  return NULL;
}
