#ifndef RUUVI_LIBRARY_DELTA_H
#define RUUVI_LIBRARY_DELTA_H
/**
 * @file ruuvi_library_delta.h
 * @author Otso Jousimaa
 * @date 2019-07-31
 * @brief Delta compression of sensor data optmized for timestamp + value and timestamp + 3 values datasets
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * 
 */

#include <stdint.h>

/** @brief Data is stored inside U32. */
typedef ruuvi_library_delta_storage_unit uint32_t;

/** @brief Struct to store 3 channels + timestamp **/
typedef struct {
  ruuvi_library_delta_storage_unit timestamp;
  ruuvi_library_delta_storage_unit value1;
  ruuvi_library_delta_storage_unit value2;
  ruuvi_library_delta_storage_unit value3;
}ruuvi_library_delta_storage_3ch_t;


/** @brief Struct to store 1 channel + timestamp **/
typedef struct {
  ruuvi_library_delta_storage_unit timestamp;
  ruuvi_library_delta_storage_unit value1;
}ruuvi_library_delta_storage_1ch_t;

#endif