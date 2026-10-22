#include <kernel.h>
#include <blkdev.h>
#include <tinysd.h>
#include "printf.h"

#define SPICONF	56
#define SPIDATA 57

static uint8_t spicf = 0xFF;

void sd_spi_raise_cs(void)
{
  spicf &= ~2;
  out(SPICONF, spicf);
}

void sd_spi_lower_cs(void)
{
  spicf |= 2;
  out(SPICONF, spicf);
}

void sd_spi_tx_byte(uint_fast8_t v)
{
  out(SPIDATA,v );
}

uint_fast8_t sd_spi_rx_byte(void)
{
  sd_spi_tx_byte(0xFF);
  return in(SPIDATA);
}

/* TODO call this appropriately after boot */
void sd_spi_clock(bool go_fast)
{
  spicf &= 0xFE;
  spicf |= go_fast;
}

