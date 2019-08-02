#ifndef RUUVI_LIBRARY_PLA_H
#define RUUVI_LIBRARY_PLA_H
#include <stdbool.h>
#include <stdint.h>
/**
 * @file ruuvi_library_pla.h
 * @author Otso Jousimaa
 * @date 2019-07-31
 * @brief Piecewise linear approximation of sensor data. 
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * 
 */

/** @brief State of 3 ch storage 
 *
 *  The state is made of 3 floats which represent measured sensor values at timestamp. 
 *  Delta represents rate of change as estimated at the beginning of samples
 *  Epsilon represents largest allowable error between estimate and measured states. 
 *  
 *  Once states differ by large enough amount or maximum time interval between samples has passed
 *  the state should be refreshed. 
 *
 **/
typedef struct {
  const uint64_t timestamp;
  const float value1;
  const float value2;
  const float value3;
  const float delta1;
  const float delta2;
  const float delta3;
  const float epsilon1;
  const float epsilon2;
  const float epsilon3;
  const uint64_t interval_max;
  const uint64_t interval_min;
}ruuvi_library_delta_state_3ch_t;



/** @brief State of 1 ch storage 
 *
 *  The state is made of 1 float which represents measured sensor values at timestamp. 
 *  Delta represents rate of change as estimated at the beginning of samples
 *  Epsilon represents largest allowable error between estimate and measured states. 
 *  
 *  Once states differ by large enough amount or maximum time interval between samples has passed
 *  the state should be refreshed. 
 *
 **/
typedef struct {
  const uint64_t timestamp;
  const float value1;
  float delta1;
  float epsilon1;
  const uint64_t interval_max;
  const uint64_t interval_min;
}ruuvi_library_delta_state_1ch_t;

/*
 * @param[in] base 
 * @return true if segment is valid, false if segment should be updated. 
 */
bool ruuvi_library_delta_state_1ch_update(const ruuvi_library_delta_state_1ch_t* const start,
                                          ruuvi_library_delta_state_1ch_t* const now);

#endif