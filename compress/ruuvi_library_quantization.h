#ifndef RUUVI_LIBRARY_QUANTIZATION_H
#define RUUVI_LIBRARY_QUANTIZATION_H
/**
 * @file quantization.h
 * @author Otso Jousimaa
 * @date 2019-07-31
 * @brief Quantize floats into ints for lossy compression
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * 
 */

#include <stdint.h>

/**
 * @brief Quantize given float into int.
 *
 * Usage:
 * @code{.c}
 * float ftemp = 25.7;
 * int32_t qtemp = ruuvi_library_quantize_f2i32(ftemp, 1);
 * // qtemp == 257
 * float fpres = 10365
 * int32_t qpres = ruuvi_library_quantize_f2i32(ftemp, -2);
 * // qpres == 104 (note rounding)
 * @endcode
 *
 * @param[in] value value to quantize
 * @param[in] exponent value is multiplied by 10^exponent before quantization
 * @return    quantized int 
 * @return    value clipped to INT32_MIN+1 or INT32_MAX if float overflows int
 * @return    RUUVI_LIBRARY_INT32_INVALID if value is NAN or INF
 */
int32_t ruuvi_library_quantize_f2i32(const float value, const int8_t exponent);

/**
 * @brief Quantize given u64 into int.
 *
 * Usage:
 * @code{.c}
 * uint64_t epoch_ms = 1564594812897;
 * int32_t qepoch = ruuvi_library_quantizeu_u642i32ftemp, -4);
 * // qtemp == 156459481 ( tens of seconds since unix epoch )
 * @endcode
 *
 * @param[in] value value to quantize
 * @param[in] exponent value is multiplied by 10^exponent before quantization
 * @return    quantized int 
 * @return    value clipped to INT32_MAX if value overflows int
 * @return    RUUVI_LIBRARY_INT32_INVALID if value is NAN or INF
 */
int32_t ruuvi_library_quantize_u642i32(const uint64_t value, const int8_t exponent);

#endif