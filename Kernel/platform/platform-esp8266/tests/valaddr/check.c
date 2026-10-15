/*
 *	User-pointer (valaddr) bounds test for the ESP8266 Fuzix port.
 *
 *	Exercises the custom valaddr() in
 *	Kernel/platform/platform-esp8266/misc.c through ordinary syscalls
 *	that copy a user buffer.
 *
 *	Fixed memory map for this port (config.h / kernel.ld):
 *
 *	    DATABASE   0x3ffe8000   start of the per-process data segment
 *	    UDATA_SIZE 1536         kernel u_data kept at the bottom of it
 *	    PROGLOAD   0x3ffe8600   first byte a user process may address
 *	    DATATOP    0x3fff8000   end of user memory (kernel DRAM starts)
 *
 *	u_top (the process's real top) sits between PROGLOAD and DATATOP;
 *	the bad addresses used below are deliberately above any small
 *	process's u_top.
 *
 *	On a fixed kernel every bad pointer fails with EFAULT.
 *	On an unfixed kernel the syscalls wrongly succeed, and the
 *	0xfffffffc case takes a physical-address fault.
 *
 *	Exit status is the number of failures (0 = all good).
 *
 *	Build:  make          (see ../README.md)
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define DATABASE   0x3ffe8000u
#define UDATA_SIZE 1536u
#define PROGLOAD   (DATABASE + UDATA_SIZE)
#define DATATOP    0x3fff8000u

static int total, fails, info;

static void check(const char *what, ssize_t ret, int err, int want_fail)
{
	int ok;

	total++;
	ok = want_fail ? (ret == -1 && err == EFAULT) : (ret >= 0);
	if (ok) {
		printf("ok    %-40s ret=%-3ld errno=%d\n",
		       what, (long)ret, err);
		return;
	}
	fails++;
	printf("FAIL  %-40s ret=%-3ld errno=%d (%s)%s\n",
	       what, (long)ret, err, strerror(err),
	       want_fail ? " [wanted EFAULT]" : " [wanted success]");
}

/* Informational only: the errno on an unfixed kernel is not worth
 * asserting (it may fault the box), so just print it. */
static void note(const char *what, ssize_t ret, int err)
{
	info++;
	printf("note  %-40s ret=%-3ld errno=%d (%s)\n",
	       what, (long)ret, err, strerror(err));
}

/* Sequence the syscall before sampling errno (argument evaluation
 * order is unspecified otherwise). */
#define TRY(what, call, want_fail) do { \
	ssize_t r_; \
	errno = 0; \
	r_ = (call); \
	check(what, r_, errno, want_fail); \
} while (0)

#define TRYNOTE(what, call) do { \
	ssize_t r_; \
	errno = 0; \
	r_ = (call); \
	note(what, r_, errno); \
} while (0)

int main(void)
{
	int p[2];
	char buf[8];
	char stackmark;			/* approximation of u_top */

	setvbuf(stdout, NULL, _IONBF, 0);

	printf("valaddr bounds test\n");
	printf("DATABASE=%p PROGLOAD=%p DATATOP=%p  stack~%p\n",
	       (void *)DATABASE, (void *)PROGLOAD, (void *)DATATOP,
	       (void *)&stackmark);

	if (pipe(p)) {
		perror("pipe");
		return 2;
	}

	/* --- baseline: valid transfers must keep working --- */
	memcpy(buf, "ABCD", 4);
	TRY("write p[1] buf 4", write(p[1], buf, 4), 0);
	TRY("read  p[0] buf 4", read(p[0], buf, 4), 0);

	/* leave data in the pipe for the read-into-bad-pointer tests */
	memcpy(buf, "WXYZ", 4);
	write(p[1], buf, 4);

	/* --- write() source pointers --- */
	TRY("write src=DATABASE (u_data)",
	    write(p[1], (void *)DATABASE, 4), 1);
	TRY("write src=PROGLOAD-1 (last u_data byte)",
	    write(p[1], (void *)(PROGLOAD - 1), 1), 1);
	TRY("write src=DATATOP-4 (above u_top)",
	    write(p[1], (void *)(DATATOP - 4), 4), 1);

	/* --- read() destination pointers --- */
	TRY("read  dst=DATABASE (u_data)",
	    read(p[0], (void *)DATABASE, 4), 1);
	TRY("read  dst=PROGLOAD-1 (last u_data byte)",
	    read(p[0], (void *)(PROGLOAD - 1), 1), 1);
	TRY("read  dst=DATATOP-4 (above u_top)",
	    read(p[0], (void *)(DATATOP - 4), 4), 1);

	/*
	 * Wrapping address: 0xfffffffc + 4 wraps to 0.  On a fixed
	 * kernel the address is out of range anyway -> EFAULT.  On an
	 * unfixed kernel valaddr() returns 4 and the kernel faults.
	 */
	TRYNOTE("write src=0xfffffffc (wrap addr)",
		write(p[1], (void *)0xfffffffcu, 4));

	/*
	 * Size that wraps: PROGLOAD + 0xffffff00 wraps the 32-bit
	 * address space, so the old "base + size < base" test set the
	 * length to MAXUSIZE-base+1 and skipped the clip against u_top.
	 * Do this last, and treat it as informational.
	 */
	TRYNOTE("write src=PROGLOAD size=0xffffff00",
		write(p[1], (void *)PROGLOAD, 0xffffff00u));

	printf("---\n%d/%d checks passed (%d informational)\n",
	       total - fails, total, info);
	return fails ? 1 : 0;
}

/* vim: set ts=4 sw=4 et */
