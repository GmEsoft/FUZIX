#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Focused check for ed append() growing its line pointer table.
 *
 * append() keeps the buffer's line numbers in zero[1..], growing the
 * table with realloc once (dol-zero)+1 reaches nlall.  The table must
 * survive repeated grows with all entries and the dot/dol pointers
 * intact.  A free() before the realloc() turns every grow into a
 * use-after-free (ASan: "attempting double-free").
 *
 * Globals mirror ed.c; putline()/error() are small stand-ins so the
 * real append() body (spliced in below) runs unmodified.
 */

int *zero;
int *dot;
int *dol;
unsigned nlall;
int lastc;

#define EOF (-1)

/* Feeder standing in for getfile/getsub: N lines, then EOF. */
static int feed_remaining;
static int feed(void)
{
	if (feed_remaining > 0) {
		feed_remaining--;
		return 0;
	}
	return EOF;
}

/* Stand-in for the real putline(): hand out distinct line numbers. */
static int tl_next;
int putline(void)
{
	return ++tl_next;
}

/* Must not fire: the table starts at 128 ints and only grows. */
void error(const char *s)
{
	fprintf(stderr, "unexpected ed error: %s\n", s);
	abort();
}

/* FUNCTION Applications/V7/cmd/ed.c append */

int main(void)
{
	int nline, i;

	/* Mirror ed main(): empty buffer, table for 128 lines. */
	nlall = 128;
	zero = malloc(nlall * sizeof(int));
	assert(zero != NULL);
	dot = dol = zero;

	/* 700 lines forces two grows (128 -> 640 -> 1152). */
	tl_next = 0;
	feed_remaining = 700;
	nline = append(feed, dot);
	assert(nline == 700);
	assert(feed_remaining == 0);

	/* The grows really happened and the table holds every line. */
	assert(nlall > 128);
	assert(dol == zero + 700);
	assert(dot == zero + 700);
	for (i = 1; i <= 700; i++)
		assert(zero[i] == i);

	free(zero);
	return 0;
}
