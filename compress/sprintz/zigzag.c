/*
uint16_t zigzagi16(const int16_t value)
{
  return ( 0 > value) ? ((0-value)<<1)-1 : (value << 1)
}

int16_t zigzagu16(const uint16_t value)
{
  return (value & 1)? 0 - ((value + 1)>>1) : value >> 1;
}
*/