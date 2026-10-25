#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* 0x2000-0xFEFF */
uint8_t header[5] = { 0x00, 0xDF, 0x00, 0x20, 0x00 };
uint8_t exec[5] = { 0xFF, 0x20, 0x00, 0x20, 0x00 };

void writex(int fd, const uint8_t *ptr, int len)
{
    if (write(fd, (char *)ptr, len) != len) {
        perror("write");
        exit(1);
    }
}

void write_decb(unsigned start, unsigned len)
{
    uint8_t buf[256];
    int l;
    int total = 0;

    header[1] = len >> 8;
    header[2] = len;
    header[3] = start >> 8;
    header[4] = start;
    writex(1, header, 5);

    if (lseek(0, start, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }
    while((l = read(0, buf, 256)) > 0) {
        writex(1, buf, l);
        total += l;
    }
    if (l < 0) {
        perror("read");
        exit(1);
    }
    if (total > len) {
        fprintf(stderr, "Image too large.\n");
        exit(1);
    }
    /* Remainder */
    len -= total;
    /* Makes it easier to see what is going on */
    memset(buf, 0xFF, 256);
    while(len) {
        l = 256;
        if (len < 256)
            l = len;
        writex(1, buf, l);
        len -= l;
    }
}

void write_tail(unsigned addr)
{
    exec[1] = addr >> 8;
    exec[3] = addr >> 8;
    exec[2] = addr;
    exec[4] = addr;
    writex(1, exec, 5);
}

int main(int argc, char *argv[])
{
    write_decb(0x1C00, 0xFE00 - 0x1C00);
    write_tail(0x1C00);
}
