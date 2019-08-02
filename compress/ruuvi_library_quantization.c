#include "ruuvi_library.h"
#include "ruuvi_library_quantization.h"
#include <math.h>
#include <stdint.h>

int32_t ruuvi_library_quantize_f2i32(const float value, const int8_t exponent)
{
  if(!isfinite(value)) { return RUUVI_LIBRARY_I32_INVALID; }
  const float f = value * powf(10, exponent);
  if(f < (INT32_MIN+1)) { return INT32_MIN+1; }
  if(f > (INT32_MAX)) { return INT32_MAX; }
  return f;
}

int32_t ruuvi_library_quantize_u642i32(const uint64_t value, const int8_t exponent)
{
  float multiplier = powf(10, exponent);
  return (int32_t) value*multiplier;
}