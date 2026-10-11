#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/gpio.h>
#include <sys/ioctl.h>
#include "../pico_ioctl.h"

int is_valid_uint8(const char *str) {
    char *endptr;
    long val = strtol(str, &endptr, 10);
    
    // check string was a number and within the range of uint8_t
    if (endptr != str && *endptr == '\0' && val >= 0 && val <= UINT8_MAX) {
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
  if (argc == 1 || strcmp(argv[1], "--help") == 0)
  {
      puts("Turn on/off GPIO pins on the pico");
      puts("Usage: picogpio <PIN> <VALUE>");
      return 0;
  }

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <pin> <value>\n", argv[0]);
    return 1;
  }

  if (!is_valid_uint8(argv[1])) {
    fprintf(stderr, "Error: <pin> must be a valid uint8_t value\n");
    return 1;
  }
  uint8_t pin = (uint8_t)strtoul(argv[1], NULL, 10);

  if (!is_valid_uint8(argv[2]) || strtoul(argv[2], NULL, 10) > 1) {
    fprintf(stderr, "Error: <value> must be 0 or 1\n");
    return 1;
  }
  uint8_t value = (uint8_t)strtoul(argv[2], NULL, 10);

  int fd = open("/dev/gpio", O_RDWR, 0);
  if (fd == -1) {
    perror("Failed to open /dev/gpio");
    exit(1);
  }
  
  struct gpio info;
  struct gpioreq gr;
  uint8_t mask = 1U << (pin & 7);
  memset(&info, 0, sizeof(info));
  info.pinmask = pin; /* GETINFO takes the pin in the first byte. */
  gr.pin = pin;

  if (ioctl(fd, GPIOC_GETINFO, &info) != 0) {
    perror("Failed to read GPIO configuration");
    close(fd);
    return 1;
  }
  gr.val = info.wmask | mask;
  if (ioctl(fd, GPIOC_SETRW, &gr) != 0) {
    perror("Failed to configure GPIO output");
    close(fd);
    return 1;
  }
  gr.val = mask;
  if (ioctl(fd, value ? GPIOC_SET : GPIOC_CLR, &gr) != 0) {
    perror("Failed to perform operation");
    close(fd);
    exit(1);
  }

  close(fd);
  return 0;
}
