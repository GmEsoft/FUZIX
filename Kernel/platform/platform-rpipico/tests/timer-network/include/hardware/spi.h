#ifndef TEST_SPI_H
#define TEST_SPI_H
#include <stdint.h>
#include <stdbool.h>
typedef struct spi_inst { unsigned id; } spi_inst_t;
extern spi_inst_t fixture_spi[2];
#define spi0 (&fixture_spi[0])
#define spi1 (&fixture_spi[1])
#define SPI_CPOL_0 0
#define SPI_CPHA_0 0
#define SPI_MSB_FIRST 0
unsigned spi_init(spi_inst_t *, unsigned);
unsigned spi_set_baudrate(spi_inst_t *,unsigned);
void spi_set_format(spi_inst_t *, unsigned, unsigned, unsigned, unsigned);
int spi_write_read_blocking(spi_inst_t *, const uint8_t *, uint8_t *, unsigned);
int spi_write_blocking(spi_inst_t *, const uint8_t *, unsigned);
int spi_read_blocking(spi_inst_t *, uint8_t, uint8_t *, unsigned);
#endif
