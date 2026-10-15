#include <libgen.h>
#include <string.h>

char *dirname(char *p)
{
    register char *s;
    register char *e;

    if (p == NULL || *p == 0)
        return ".";
    e = p + strlen(p) - 1;
    while (e > p && *e == '/')
        e--;
    s = e;
    while (s > p && *s != '/')
        s--;
    if (*s != '/')
        return ".";
    while (s > p && s[-1] == '/')
        s--;
    if (s == p)
        return "/";
    *s = 0;
    return p;
}
