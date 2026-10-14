/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Kept from the old malloc package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void *calloc(size_t elm, size_t sz)
{
	size_t v;
	void *ptr;
	if (sz && elm > (size_t)-1 / sz) {
		errno = ENOMEM;
		return NULL;
	}
	v = elm * sz;
	ptr = malloc(v);

	if (ptr)
		memset(ptr, 0, v);
	return ptr;
}
