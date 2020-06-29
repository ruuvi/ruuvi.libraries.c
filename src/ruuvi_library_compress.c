/**
 * Compression library implementation on nRF52832.
 *
 * License: BSD-3
 * Author: Oleg Protasevich
 */
#include "ruuvi_library_compress.h"

#define  LZF_NO_RESULT                      0
#define  BLOCK_NUM_INVALID                  0xff
#define  TIME_INVALID                       0xffffffff

typedef struct
{
    bool flag_compress_block_have_data;
    bool flag_decompress_block_have_data;
    uint8_t decompress_block[RL_COMPRESS_BLOCK_SIZE_MAX];
    uint8_t compress_block[RL_COMPRESS_BLOCK_SIZE_MIN];
    uint32_t blocks_size[RL_COMPRESS_BLOCK_NUM];
    uint32_t blocks_compressed_size[RL_COMPRESS_BLOCK_NUM];
    uint8_t * blocks_address[RL_COMPRESS_BLOCK_NUM];
    uint16_t blocks_payload_counter[RL_COMPRESS_BLOCK_NUM];
} rl_compress_block_t;

typedef struct
{
    uint8_t * address;
    uint32_t offset;
    uint32_t dec_offset;
    uint32_t size;
    bool flag_compressed;
    rl_compress_block_t block;
} rl_compress_over_block_t;

static rl_compress_over_block_t over_block[RL_COMPRESS_OVER_BLOCK_NUM];

static uint8_t get_block_null (rl_compress_over_block_t * p_over_block)
{
    uint8_t block_num = BLOCK_NUM_INVALID;

    for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
    {
        if (NULL == p_over_block->block.blocks_address[i])
        {
            block_num = i;
            break;
        }
    }

    return block_num;
}

static uint8_t get_block_num (uint8_t * p_block,
                              rl_compress_over_block_t * p_over_block)
{
    uint8_t block_num = BLOCK_NUM_INVALID;

    for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
    {
        if (p_block == p_over_block->block.blocks_address[i])
        {
            block_num = i;
            break;
        }
    }

    return block_num;
}

static uint8_t get_over_block_num (uint8_t * p_block,
                                   size_t block_size,
                                   bool new_block)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint8_t block_null = BLOCK_NUM_INVALID;
    uint16_t size;

    for (uint8_t i = 0; i < RL_COMPRESS_OVER_BLOCK_NUM; i++)
    {
        if (NULL != over_block[i].address)
        {
            if (false == over_block[i].flag_compressed)
            {
                size = over_block[i].size;
            }
            else
            {
                size = over_block[i].offset;
            }

            if ( ( (p_block + block_size) <= over_block[i].address) ||
                    (p_block >= (over_block[i].address + size)))
            {
                block_num = BLOCK_NUM_INVALID;
            }
            else
            {
                block_num = i;
                break;
            }
        }
        else
        {
            block_null = i;
        }
    }

    if (block_num == BLOCK_NUM_INVALID)
    {
        if (true == new_block)
        {
            if (block_null != BLOCK_NUM_INVALID)
            {
                block_num = block_null;
                over_block[block_num].offset = 0;
                over_block[block_num].address = p_block;
                over_block[block_num].size = (uint32_t) block_size;
            }
        }
    }

    return block_num;
}

static ret_type_t validate_block (uint8_t * p_block,
                                  size_t block_size,
                                  rl_compress_over_block_t * p_over_block)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;

    for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
    {
        if (NULL != p_over_block->block.blocks_address[i])
        {
            if ( ( (p_block + block_size) <= p_over_block->block.blocks_address[i]) ||
                    (p_block >= (p_over_block->block.blocks_address[i] +
                                 p_over_block->block.blocks_compressed_size[i])))
            {
                status += RL_COMPRESS_SUCCESS;
            }
            else
            {
                status += RL_COMPRESS_ERROR_INVALID_STATE;
                break;
            }
        }
    }

    return status;
}

static ret_type_t get_block_compressed_size (uint8_t * block,
        size_t block_size,
        size_t * compessed_size)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint8_t over_block_num;

    if ( (NULL == block) ||
            (NULL == compessed_size))
    {
        status = RL_COMPRESS_ERROR_NULL;
    }
    else
    {
        over_block_num = get_over_block_num (block, block_size, false);

        if (over_block_num == BLOCK_NUM_INVALID)
        {
            status = RL_COMPRESS_ERROR_INVALID_STATE;
        }
        else
        {
            block_num = get_block_num (block, &over_block[over_block_num]);

            if (block_num == BLOCK_NUM_INVALID)
            {
                status = RL_COMPRESS_ERROR_INVALID_STATE;
            }
            else
            {
                (*compessed_size) =
                    (size_t) over_block[over_block_num].block.blocks_compressed_size[block_num];
            }
        }
    }

    return status;
}

static void update_over_block_offset (uint8_t block_num)
{
    size_t compressed_size = 0;
    uint32_t offset = 0;
    uint32_t dec_offset = 0;

    for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
    {
        if (RL_COMPRESS_SUCCESS == get_block_compressed_size (
                    over_block[block_num].block.blocks_address[i],
                    over_block[block_num].size,
                    &compressed_size))
        {
            offset += compressed_size;
        }
        else
        {
            break;
        }
    }

    for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
    {
        if (NULL != over_block[block_num].block.blocks_address[i])
        {
            dec_offset += (over_block[block_num].block.blocks_address[i] -
                           over_block[block_num].address);
            break;
        }
    }

    over_block[block_num].dec_offset = dec_offset;
    over_block[block_num].offset = offset;
}

static void rl_data_round (rl_data_t * p_data)
{
    for (uint8_t i = 0; i < RL_COMPRESS_PAYLOAD_SIZE; i++)
    { p_data->payload[i] = (float) ( (int) (p_data->payload[i])); }
}


static ret_type_t find_data_and_remove (uint8_t block_num, rl_data_t * p_data,
                                        rl_data_t p_blocks[],
                                        timestamp_t * start_timestamp,
                                        rl_compress_over_block_t * p_over_block)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    timestamp_t min_time = TIME_INVALID;
    uint16_t array_offset = 0;
    uint16_t array_size =
        (uint16_t) (p_over_block->block.blocks_size[block_num] / RL_COMPRESS_DATA_SIZE);

    for (uint16_t i = 0; i < array_size; i++)
    {
        if (p_blocks[i].time >= (*start_timestamp))
        {
            if (p_blocks[i].time < min_time)
            {
                min_time = p_blocks[i].time;
                array_offset = i;
            }
        }
    }

    if (min_time != TIME_INVALID)
    {
        memcpy (p_data, &p_blocks[array_offset], RL_COMPRESS_DATA_SIZE);

        if (array_offset != (array_size - 1))
        {
            memcpy (&p_blocks[array_offset], &p_blocks[array_offset + 1],
                    ( (array_size - array_offset - 1) *RL_COMPRESS_DATA_SIZE));
        }

        (*start_timestamp) = min_time;
    }
    else
    {
        status = RL_COMPRESS_ERROR_INVALID_PARAM;
    }

    return status;
}

static ret_type_t rl_compress_block (rl_data_t * data,
                                     uint8_t * block,
                                     size_t block_size,
                                     rl_compress_state_t * state,
                                     rl_compress_over_block_t * p_over_block)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint32_t len;

    if (RL_COMPRESS_SUCCESS == validate_block (block, block_size, p_over_block))
    {
        block_num = get_block_num (block, p_over_block);

        if (block_num == BLOCK_NUM_INVALID)
        {
            block_num = get_block_null (p_over_block);

            if (block_num != BLOCK_NUM_INVALID)
            {
                p_over_block->block.blocks_size[block_num] = (uint32_t) block_size;
                p_over_block->block.blocks_address[block_num] = block;
                p_over_block->block.blocks_compressed_size[block_num] = 0;
                p_over_block->block.blocks_payload_counter[block_num] = 0;
            }
        }

        if (block_num != BLOCK_NUM_INVALID)
        {
            if (0 == p_over_block->block.blocks_compressed_size[block_num])
            {
                if (p_over_block->block.blocks_payload_counter[block_num] <
                        (p_over_block->block.blocks_size[block_num] / RL_COMPRESS_DATA_SIZE))
                {
#ifdef RL_COMPRESS_CONVERT_TO_INT
                    rl_data_round (data);
#endif
#if 0
                    memcpy ( (block +
                              (p_over_block->block.blocks_payload_counter[block_num]*RL_COMPRESS_DATA_SIZE)),
                             data,
                             RL_COMPRESS_DATA_SIZE);
#else
                    memcpy ( ( (uint8_t *) &p_over_block->block.compress_block[0] +
                               (p_over_block->block.blocks_payload_counter[block_num]*RL_COMPRESS_DATA_SIZE)),
                             data,
                             RL_COMPRESS_DATA_SIZE);
#endif
                    p_over_block->block.flag_compress_block_have_data = true;
                    p_over_block->block.blocks_payload_counter[block_num]++;
                }

                if (p_over_block->block.blocks_payload_counter[block_num] >=
                        p_over_block->block.blocks_size[block_num] / RL_COMPRESS_DATA_SIZE)
                {
                    rl_data_t uncomressed_block[RL_COMPRESS_BLOCK_SIZE_MAX / RL_COMPRESS_DATA_SIZE];
                    uint32_t compess_len = (p_over_block->block.blocks_size[block_num] >
                                            RL_COMPRESS_SIZE_LIMIT) ?
                                           (p_over_block->block.blocks_size[block_num] - RL_COMPRESS_SIZE_LIMIT) :
                                           (p_over_block->block.blocks_size[block_num]);
                    memcpy (&uncomressed_block,
                            p_over_block->block.compress_block,
                            p_over_block->block.blocks_size[block_num]);
                    len = (uint32_t) lzf_compress ( (u8 *) uncomressed_block,
                                                    (unsigned int) p_over_block->block.blocks_size[block_num],
                                                    (u8 *) (p_over_block->block.compress_block),
                                                    (unsigned int) compess_len,
                                                    (const u8 **) state);

                    if (LZF_NO_RESULT == len)
                    {
                        status = RL_COMPRESS_ERROR_INVALID_STATE;
                    }
                    else
                    {
                        p_over_block->block.flag_compress_block_have_data = false;

                        if ( (p_over_block->offset + len) <= p_over_block->size)
                        {
                            memcpy (block,
                                    p_over_block->block.compress_block,
                                    len);
                            p_over_block->block.blocks_compressed_size[block_num] = len;

                            if (0 < block_num)
                            {
                                if ( (p_over_block->block.blocks_address[block_num] ==
                                        (p_over_block->block.blocks_address[block_num - 1] +
                                         p_over_block->block.blocks_compressed_size[block_num - 1])))
                                {
                                    if (RL_COMPRESS_BLOCK_SIZE_MAX >=
                                            (p_over_block->block.blocks_size[block_num - 1] +
                                             p_over_block->block.blocks_size[block_num]))
                                    {
                                        p_over_block->block.blocks_compressed_size[block_num - 1] +=
                                            p_over_block->block.blocks_compressed_size[block_num];
                                        p_over_block->block.blocks_size[block_num - 1] +=
                                            p_over_block->block.blocks_size[block_num];
                                        p_over_block->block.blocks_payload_counter[block_num - 1] +=
                                            p_over_block->block.blocks_payload_counter[block_num];
                                        p_over_block->block.blocks_size[block_num] = 0;
                                        p_over_block->block.blocks_compressed_size[block_num] = 0;
                                        p_over_block->block.blocks_address[block_num] = NULL;
                                        p_over_block->block.blocks_payload_counter[block_num] = 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            p_over_block->block.blocks_size[block_num] = 0;
                            p_over_block->block.blocks_compressed_size[block_num] = 0;
                            p_over_block->block.blocks_address[block_num] = NULL;
                            p_over_block->block.blocks_payload_counter[block_num] = 0;
                            p_over_block->flag_compressed = true;
                            status = RL_COMPRESS_END;
                        }

                        memset (p_over_block->block.compress_block, 0,
                                p_over_block->block.blocks_size[block_num]);
                    }
                }
            }
            else
            {
                status = RL_COMPRESS_ERROR_INVALID_STATE;
            }
        }
        else
        {
            status = RL_COMPRESS_ERROR_INVALID_STATE;
        }
    }
    else
    {
        status = RL_COMPRESS_ERROR_INVALID_STATE;
    }

    return status;
}

static ret_type_t rl_decompress_block (rl_data_t * data,
                                       uint8_t * block,
                                       size_t block_size,
                                       rl_compress_state_t * state,
                                       timestamp_t * start_timestamp,
                                       rl_compress_over_block_t * p_over_block)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    block_num = get_block_num (block, p_over_block);

    if (block_num != BLOCK_NUM_INVALID)
    {
        rl_data_t uncomressed_block[RL_COMPRESS_BLOCK_SIZE_MAX / RL_COMPRESS_DATA_SIZE];
        memset (uncomressed_block, 0, RL_COMPRESS_BLOCK_SIZE_MAX);

        if (true == p_over_block->block.flag_compress_block_have_data)
        {
            memcpy (&p_over_block->block.decompress_block[0],
                    (uint8_t *) &p_over_block->block.compress_block[0],
                    p_over_block->block.blocks_size[block_num]);
            p_over_block->block.flag_compress_block_have_data = false;
        }

        if (0 != p_over_block->block.blocks_compressed_size[block_num])
        {
            memcpy ( (uint8_t *) &p_over_block->block.decompress_block[0],
                     (uint8_t *) &block[0],
                     p_over_block->block.blocks_compressed_size[block_num]);

            if (LZF_NO_RESULT == lzf_decompress ( (u8 *) (uint8_t *)
                                                  &p_over_block->block.decompress_block[0],
                                                  p_over_block->block.blocks_compressed_size[block_num],
                                                  (u8 *) uncomressed_block, p_over_block->block.blocks_size[block_num]))
            {
                status = RL_COMPRESS_ERROR_INVALID_STATE;
            }
        }
        else
        {
            memcpy (&uncomressed_block,
                    (uint8_t *) &p_over_block->block.decompress_block[0],
                    p_over_block->block.blocks_size[block_num]);
        }

        if (status == RL_COMPRESS_SUCCESS)
        {
            status = find_data_and_remove (block_num, data, uncomressed_block,
                                           start_timestamp, p_over_block);

            if (status == RL_COMPRESS_SUCCESS)
            {
                memcpy ( (uint8_t *) &p_over_block->block.decompress_block[0],
                         &uncomressed_block,
                         p_over_block->block.blocks_size[block_num]);
                p_over_block->block.flag_decompress_block_have_data = true;
                p_over_block->block.blocks_compressed_size[block_num] = 0;
                p_over_block->block.blocks_payload_counter[block_num]--;
                p_over_block->flag_compressed = false;

                if (0 == p_over_block->block.blocks_payload_counter[block_num])
                {
                    p_over_block->block.blocks_size[block_num] = 0;
                    p_over_block->block.blocks_address[block_num] = NULL;
                    p_over_block->block.flag_decompress_block_have_data = false;

                    if ( (p_over_block->dec_offset + RL_COMPRESS_BLOCK_SIZE_DEFAULT) >= p_over_block->size)
                    {
                        p_over_block->address = NULL;
                        p_over_block->size = 0;
                        status = RL_COMPRESS_END;
                    }
                }
            }
        }
    }
    else
    {
        status = RL_COMPRESS_ERROR_INVALID_STATE;
    }

    return status;
}

ret_type_t rl_get_compressed_size (uint8_t * block,
                                   size_t block_size,
                                   size_t * compessed_size)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint8_t over_block_num;

    if ( (NULL == block) ||
            (NULL == compessed_size))
    {
        status = RL_COMPRESS_ERROR_NULL;
    }
    else
    {
        over_block_num = get_over_block_num (block, block_size, false);

        if (over_block_num == BLOCK_NUM_INVALID)
        {
            status = RL_COMPRESS_ERROR_INVALID_STATE;
        }
        else
        {
            (*compessed_size) = (size_t) over_block[over_block_num].offset;
        }
    }

    return status;
}

ret_type_t rl_compress (rl_data_t * data,
                        uint8_t * block,
                        size_t block_size,
                        rl_compress_state_t * state)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t over_block_num;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint16_t decompress_counter = 0;
    uint16_t payload_counter = 0;

    if ( (NULL == data) ||
            (NULL == block) ||
            (NULL == state))
    {
        status = RL_COMPRESS_ERROR_NULL;
    }
    else
    {
        if ( (RL_COMPRESS_OVER_BLOCK_SIZE_MAX < block_size) ||
                (RL_COMPRESS_OVER_BLOCK_SIZE_MIN > block_size) ||
                ( (block_size % RL_COMPRESS_SIZE_LIMIT) != 0))
        {
            status = RL_COMPRESS_ERROR_INVALID_PARAM;
        }
        else
        {
            over_block_num = get_over_block_num (block, block_size, true);

            if (over_block_num != BLOCK_NUM_INVALID)
            {
                if (true == over_block[over_block_num].block.flag_decompress_block_have_data)
                {
                    block_num = get_block_num (block, &over_block[over_block_num]);

                    if (block_num != BLOCK_NUM_INVALID)
                    {
                        payload_counter = (over_block[over_block_num].block.blocks_payload_counter[block_num] *
                                           RL_COMPRESS_DATA_SIZE);
                        over_block[over_block_num].block.blocks_payload_counter[block_num] = 0;
                    }

                    while (payload_counter > decompress_counter)
                    {
                        status = rl_compress_block ( (rl_data_t *) ( (uint8_t *)
                                                     &over_block[over_block_num].block.decompress_block[decompress_counter]),
                                                     (over_block[over_block_num].address +
                                                      over_block[over_block_num].offset),
                                                     RL_COMPRESS_BLOCK_SIZE_DEFAULT,
                                                     state,
                                                     &over_block[over_block_num]);
                        update_over_block_offset (over_block_num);
                        decompress_counter += RL_COMPRESS_DATA_SIZE;
                    }

                    over_block[over_block_num].block.flag_decompress_block_have_data = false;
                }

                status = rl_compress_block (data,
                                            (over_block[over_block_num].address +
                                             over_block[over_block_num].offset),
                                            RL_COMPRESS_BLOCK_SIZE_DEFAULT,
                                            state,
                                            &over_block[over_block_num]);
                update_over_block_offset (over_block_num);
            }
            else
            {
                status = RL_COMPRESS_ERROR_INVALID_STATE;
            }
        }
    }

    return status;
}

ret_type_t rl_decompress (rl_data_t * data,
                          uint8_t * block,
                          size_t block_size,
                          rl_compress_state_t * state,
                          timestamp_t * start_timestamp)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t over_block_num;

    if ( (NULL == data) ||
            (NULL == block) ||
            (NULL == state) ||
            (NULL == start_timestamp))
    {
        status = RL_COMPRESS_ERROR_NULL;
    }
    else
    {
        if ( (RL_COMPRESS_OVER_BLOCK_SIZE_MAX < block_size) ||
                (RL_COMPRESS_OVER_BLOCK_SIZE_MIN > block_size))
        {
            status = RL_COMPRESS_ERROR_INVALID_PARAM;
        }
        else
        {
            over_block_num = get_over_block_num (block, block_size, false);

            if (over_block_num != BLOCK_NUM_INVALID)
            {
                status = rl_decompress_block (data,
                                              (over_block[over_block_num].address +
                                               over_block[over_block_num].dec_offset),
                                              block_size,
                                              state,
                                              start_timestamp,
                                              &over_block[over_block_num]);
                update_over_block_offset (over_block_num);
            }
            else
            {
                status = RL_COMPRESS_ERROR_INVALID_STATE;
            }
        }
    }

    return status;
}