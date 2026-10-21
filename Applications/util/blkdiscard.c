#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define perror_exit(s) \
	do { perror(s); exit(1); } while (0)

static bool force = false;
static const char* filename;
static int fd;
static uint32_t size;

static void syntax_error(void)
{
	fprintf(stderr, "Usage: blkdiscard [-f] device\n");
	exit(1);
}

int main(int argc, char* const* argv)
{
	for (;;)
	{
		int opt = getopt(argc, argv, "f");
		if (opt == -1)
			break;

		switch (opt)
		{
			case 'f':
				force = true;
				break;

			default:
				syntax_error();
		}
	}
	if (optind != (argc-1))
		syntax_error();
	filename = argv[optind];

	fd = open(filename, O_RDWR);
	if (fd == -1)
		perror_exit("cannot open block device");

	if (ioctl(fd, BLKGETSIZE, &size) == -1)
		perror_exit("cannot get size of partition");

	if (!force)
	{
		printf("This will destroy %lukB of data on %s. Proceed? ",
			(unsigned long)size / 2, filename);
		fflush(stdout);
		if (getchar() != 'y')
		{
			printf("Aborted.\n");
			exit(1);
		}
	}

	printf("Erasing...\n");
	fflush(stdout);
	{
		uint32_t blkno;
		uint16_t lba;

		/* HDIO_TRIM takes a 16-bit block number, so a bigger
		 * device cannot be addressed by this interface. */
		if (size > 65536UL)
		{
			fprintf(stderr,
				"%s: device is too large to discard\n",
				filename);
			exit(1);
		}

		for (blkno=0; blkno<size; blkno++)
		{
			lba = blkno;
			if (ioctl(fd, HDIO_TRIM, &lba) == -1)
				perror_exit("erase failed");
		}
	}

	printf("Done.\n");
	close(fd);
	return 0;
}

