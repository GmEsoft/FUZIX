#include <math.h>

uint32_t __float_bits(float p)
{
  union {
    float f;
    uint32_t bits;
  } conv;
  
  conv.f = p;
  return conv.bits;
}
