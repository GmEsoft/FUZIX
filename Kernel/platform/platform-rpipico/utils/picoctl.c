#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../pico_ioctl.h"

int main(int argc, char **argv)
{
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0))
    {
        puts("usage: picoctl [ --help ] <command>");
        puts("Command list:");
        puts("\tflash\tReset into flash mode.");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "flash") != 0)
    {
        fputs("usage: picoctl [ --help ] flash\n", stderr);
        return 1;
    }
    int fd = open("/dev/sys", O_RDWR, 0);
    if (fd == -1)
    {
        perror("Failed to open /dev/sys");
        exit(1);
    }
    if (ioctl(fd, PICOIOC_FLASH) != 0)
    {
        perror("Failed to perform operation");
        close(fd);
        exit(1);
    }
    close(fd);
    return 0;
}
