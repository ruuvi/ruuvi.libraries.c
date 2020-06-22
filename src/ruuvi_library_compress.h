#ifndef RUUVI_LIBRARY_COMPRESS_H
#define RUUVI_LIBRARY_COMPRESS_H
/**
* @file ruuvi_library_compress.h
* @author Oleg Protasevich
* @date 2020-06-16
* @brief Compression API
* @copyright Copyright 2020 Ruuvi Innovations.
*   This project is released under the BSD-3-Clause License.
*
*
*/

#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include "liblzf-3.6/lzf.h"

//#define RL_COMPRESS_CONVERT_TO_INT

#define RL_COMPRESS_SUCCESS                (0U)       ///< Internal Error
#define RL_COMPRESS_ERROR_INVALID_PARAM    (1U<<4U)   ///< Invalid Parameter
#define RL_COMPRESS_ERROR_INVALID_STATE    (1U<<5U)   ///< Invalid state, operation disallowed in this state
#define RL_COMPRESS_ERROR_NULL             (1U<<11U)  ///< Null Pointer

#define RL_COMPRESS_STATE_SIZE              1 << (HLOG)
#define RL_COMPRESS_PAYLOAD_SIZE            3

typedef uint32_t timestamp_t;
typedef uint32_t ret_type_t;  ///< bitfield for representing errors
typedef LZF_HSLOT rl_compress_state_t[RL_COMPRESS_STATE_SIZE];

#pragma pack(push, 1)
typedef struct
{
    timestamp_t time;
    float payload[RL_COMPRESS_PAYLOAD_SIZE];
} rl_data_t;
#pragma pack(pop)

/**
 * @brief Ruuvi Library compress function.
 * Takes a sensor data sample in and appends it to given data block.
 * It can be assumed that data is appended in linear order, new sample has always greater timestamp than previous.
 *
 * @param[in] data Sensor data to compress.
 * @param[in] block Pointer to buffer which has sensor data.
 * @param[in] block_size Size of block.
 * @param[in,out] state In: State of compression algorithm before adding latest data. Out: State of compression algorithm after adding latest data.
 * @return status code indicating if compression was successful.
 *
 */
ret_type_t rl_compress (rl_data_t * data,
                        uint8_t * block,
                        size_t block_size,
                        rl_compress_state_t * state);
/**
 * @brief Ruuvi Library decompress function.
 * Looks up next sample after given timestamp and returns it via output parameter.
 *
 *
 * @param[out] data Next sample from block. Not modified if no data was found in block.
 * @param[in]  block Pointer to compressed buffer with sensor data.
 * @param[in]  block_size Size of block.
 * @param[in,out] state In: State of decompression algorithm before decompressing next data point. Out: State of decompression algorithm after decompressing next data point.
 * @param[in,out] start_timestamp In: Earliest timestamp to accept. Out: Timestamp of returned data.
 * @return Status of decompression, such as more available or not_found.
 *
 * Usage:
 * ret_type_t status;
 * rl_data_t data;
 * uint8_t* block = get_next_block();
 * size_t block_size = DATA_BLOCK_SIZE;
 * rl_decompress_state_t state = { 0 }; // zeroed state initially.
 * timestamp_t start_timestamp = 0; // Get all the data from boot.
 * while(RL_COMPRESS_SUCCESS == status)
 * {
 *   // This pseudocode fails if no data is found and it misses last sample.
 *   status = rl_decompress(&data, block, block_size, &state, &start_timestamp);
 *   do_something_with_data(&data);
 * }
 *
 *
 */
ret_type_t rl_decompress (rl_data_t * data,
                          uint8_t * block,
                          size_t block_size,
                          rl_compress_state_t * state,
                          timestamp_t * start_timestamp);

#endif