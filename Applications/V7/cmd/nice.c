/* From UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/* nice */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int nicarg = 10;
	int exit_status = 126;

	if (argc > 1 && argv[1][0] == '-') {
		if (argv[1][1] == 'n') {
			/* -n, -n<num> or -n <num> (bare -n means 10) */
			if (argv[1][2])
				nicarg = atoi(&argv[1][2]);
			else if (argc > 2 &&
				 (argv[2][0] == '-' ||
				  (argv[2][0] >= '0' && argv[2][0] <= '9')))
				nicarg = atoi(argv[2]);
			else
				goto run;	/* bare -n, keep 10 */
			argc--;
			argv++;		/* also the numeric argument */
		} else
			nicarg = atoi(&argv[1][1]);
run:
		argc--;
		argv++;
	}
	if(argc < 2) {
		fputs("usage: nice [ -n ] command\n", stderr);
		exit(125);
	}
	nice(nicarg);
	execvp(argv[1], &argv[1]);
	if (errno == ENOENT)	/* not found: 127, found but not
					   runnable: 126, per shell convention */
		exit_status = 127;
	perror(argv[1]);
	exit(exit_status);
}
