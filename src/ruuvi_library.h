#ifndef RUUVI_LIBRARY_H
#define RUUVI_LIBRARY_H

/**
 * @file ruuvi_library.h
 * @author Otso Jousimaa
 * @date 2019-07-22
 * @brief Common definitions for libraries
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 */
#include <stdint.h>

#define RUUVI_LIBRARY_SEMVER "0.1.2"

#define RUUVI_LIBRARY_SUCCESS 0                 //!< Success
#define RUUVI_LIBRARY_ERROR_INTERNAL    (1<<0)  //!< Unknown error, some failed assumption. 
#define RUUVI_LIBRARY_ERROR_NO_MEM      (1<<1)  //!< Not enough memory for operation
#define RUUVI_LIBRARY_ERROR_CONCURRENCY (1<<2)  //!< Failed to acquire mutex
#define RUUVI_LIBRARY_ERROR_NULL        (1<<3)  //!< Unexpected NULL pointer
#define RUUVI_LIBRARY_ERROR_NO_DATA     (1<<4)  //!< Unxpected end of data
#define RUUVI_LIBRARY_ERROR_SELFTEST    (1<<5)  //!< Self-test noticed invalid output
#define RUUVI_LIBRARY_ERROR_DATA_LENGTH (1<<6)  //!< Data length is invalid for any reason
#define RUUVI_LIBRARY_ERROR_FATAL       (1<<31) //!< Unrecoverable error, restart program

#define RUUVI_LIBRARY_I32_INVALID       INT32_MIN

typedef int32_t ruuvi_library_status_t;





#endif