/**
 * Compression library implementation on nRF52832.
 *
 * License: BSD-3
 * Author: Oleg Protasevich
 */
#include "ruuvi_library_compress.h"

#define  LZF_NO_RESULT                      0


/*
typedef struct
{
    LZF_HSLOT algo_state; //!< Hashtable, memset to 0 to reset. 4 kB
    uint8_t compress_block[RL_COMPRESS_COMPRESS_SIZE]; // Compressed block, 4 kB
    uint8_t decompress_block[RL_COMPRESS_DECOMPRESS_SIZE]; // Decompressed block, 8 kB
    size_t compressed_size;   //!< Number of compressed bytes in compress block.
    size_t decompressed_size; //!< Number of uncompressed bytes in decompress block.
}rl_compress_state_t;
*/

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
ret_type_t rl_compress (const rl_data_t * const data,
                        uint8_t * const block,
                        const size_t block_size,
                        rl_compress_state_t * const state)
{
    ret_type_t err_code = RL_COMPRESS_SUCCESS;
    const unsigned int compress_free = block_size - state->compressed_size;
    const unsigned int decompress_free = RL_COMPRESS_DECOMPRESS_SIZE - state->decompressed_size;
    if(NULL == data || NULL == block || NULL == state)
    {
      err_code |= RL_COMPRESS_ERROR_NULL;
    }
    else if(sizeof(rl_data_t) > decompress_free)
    {
      err_code |= RL_COMPRESS_LIMIT_REACHED; //!< Maximum compression ratio of 50% reached.
    }
    else
    {
      unsigned int cmpr_len = 0; //!< number of bytes compressed.
      const uint8_t* const p_block = (block + state->compressed_size); //!< Pointer to first free byte in compressed data.
      
      cmpr_len = lzf_compress (data,  sizeof(rl_data_t),
                               p_block, size_free,
                               state->algo_state); // Compress
      // Compression failure, we're ready. 
      if(0U == cmpr_len)
      {
          err_code |= RL_COMPRESS_END;
      }
      else
      {
          // Update state.
          state->compressed_size += cmpr_len; 
          memcpy((void*)(state->decompress_block + state->decompress_size,
                  data, sizeof(rl_data_t));
          state->decompressed_size += sizeof(rl_data_t);
      }
    }
    return err_code;
}

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
                          timestamp_t * start_timestamp)
{
    return status;
}