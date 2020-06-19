/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * Compression library implementation on nRF52832.
 *
 * License: BSD-3
 * Author: Oleg Protasevich
 */
#include "ruuvi_library_compress.h"

#define  LZF_NO_RESULT                      0
#define  RL_COMPRESS_SIZE_LIMIT             sizeof(uint32_t)
#define  RL_COMPRESS_DATA_SIZE              sizeof(rl_data_t)
#define  RL_COMPRESS_BLOCK_SIZE_MAX         4096
#define  RL_COMPRESS_BLOCK_SIZE_MIN         64
#define  RL_COMPRESS_BLOCK_NUM              16

#define  BLOCK_NUM_INVALID                  0xff
#define  TIME_INVALID                       0xffffffff

static uint32_t blocks_size[RL_COMPRESS_BLOCK_NUM];
static uint32_t blocks_compressed_size[RL_COMPRESS_BLOCK_NUM];
static uint8_t *blocks_address[RL_COMPRESS_BLOCK_NUM];
static uint8_t blocks_payload_counter[RL_COMPRESS_BLOCK_NUM];

static uint8_t get_block_null(void)
{
  uint8_t block_num = BLOCK_NUM_INVALID;
  for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
  {
    if (NULL == blocks_address[i])
    {
      block_num = i;
      break;
    }
  }
  return block_num;
}

static uint8_t get_block_num(uint8_t * p_block,
                             size_t block_size)
{
  uint8_t block_num = BLOCK_NUM_INVALID;
  for (uint8_t i = 0; i < RL_COMPRESS_BLOCK_NUM; i++)
  {
    if (p_block == blocks_address[i])
    {
      block_num = i;
      break;
    }
  }
  return block_num;
}

static void rl_data_round(rl_data_t *p_data)
{
  for (uint8_t i = 0; i < RL_COMPRESS_PAYLOAD_SIZE; i++)
    p_data->payload[i] = (float)((int)(p_data->payload[i]));
}


static ret_type_t find_data_and_remove(uint8_t block_num, rl_data_t *p_data, 
                                       rl_data_t p_blocks[], 
                                       timestamp_t *start_timestamp)
{
  ret_type_t status = RL_COMPRESS_SUCCESS;
  timestamp_t min_time = TIME_INVALID;
  uint16_t array_offset = 0;
  uint16_t array_size = (uint16_t)(blocks_size[block_num]/RL_COMPRESS_DATA_SIZE);
  for (uint16_t i = 0; i< array_size; i++)
  {
    if (p_blocks[i].time >= (*start_timestamp)){
      if (p_blocks[i].time < min_time)
      {
       min_time = p_blocks[i].time;
       array_offset = i;
      }
    }
  }
  if  (min_time != TIME_INVALID)
  {
    memcpy(p_data, &p_blocks[array_offset], RL_COMPRESS_DATA_SIZE);
    if (array_offset != (array_size -1))
    {
      memcpy(&p_blocks[array_offset],&p_blocks[array_offset + 1], 
             ((array_size - array_offset - 1)*RL_COMPRESS_DATA_SIZE));
    }
    (*start_timestamp) = min_time;
  }
  else
  {
    status = RL_COMPRESS_ERROR_INVALID_PARAM;
  }
  return status;
}

ret_type_t rl_compress(rl_data_t * data, 
                       uint8_t * block,   
                       size_t block_size,
                       rl_compress_state_t *state)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint32_t len;
    if ((NULL == data) ||
        (NULL == block) ||
        (NULL == state))
    {
      status = RL_COMPRESS_ERROR_NULL;
    }
    else 
    {
      if ((RL_COMPRESS_BLOCK_SIZE_MAX < block_size) ||
         (RL_COMPRESS_BLOCK_SIZE_MIN > block_size) ||
         ((block_size%RL_COMPRESS_SIZE_LIMIT) != 0)) 
      {
            status = RL_COMPRESS_ERROR_INVALID_PARAM;
      }
      else
      {
        block_num = get_block_num(block,block_size);
        if (block_num == BLOCK_NUM_INVALID)
        {
          block_num = get_block_null();
          if (block_num != BLOCK_NUM_INVALID)
          {
            blocks_size[block_num] = (uint32_t)block_size;
            blocks_address[block_num] = block;
            blocks_compressed_size[block_num] = 0;
          }
        }
        if (block_num != BLOCK_NUM_INVALID)
        {
          if (0 == blocks_compressed_size[block_num])
          {
            if (blocks_payload_counter[block_num] < (blocks_size[block_num]/RL_COMPRESS_DATA_SIZE))
            {
#ifdef RL_COMPRESS_CONVERT_TO_INT
              rl_data_round(data);
#endif
              memcpy((block + (blocks_payload_counter[block_num]*RL_COMPRESS_DATA_SIZE)),
                      data, 
                      RL_COMPRESS_DATA_SIZE);
              blocks_payload_counter[block_num]++;
            }

            if (blocks_payload_counter[block_num] >= blocks_size[block_num]/RL_COMPRESS_DATA_SIZE)
            {
              uint8_t uncomressed_block[RL_COMPRESS_BLOCK_SIZE_MAX];
              uint32_t compess_len = (blocks_size[block_num] > RL_COMPRESS_SIZE_LIMIT) ? 
                                     (blocks_size[block_num] - RL_COMPRESS_SIZE_LIMIT) : 
                                     (blocks_size[block_num]);
              memcpy(&uncomressed_block,
                      block, 
                      blocks_size[block_num]);

              len = (uint32_t)lzf_compress ((u8 *)uncomressed_block,  
                                            (unsigned int)blocks_size[block_num],
                                            (u8 *)(block),
                                            (unsigned int)compess_len, 
                                            (const u8**)state);
              if (LZF_NO_RESULT == len)
              {
                  status = RL_COMPRESS_ERROR_INVALID_STATE;
              }
              else
              {
                  blocks_compressed_size[block_num] = len;
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
    }
    return status;
}

ret_type_t rl_decompress(rl_data_t * data, 
                         uint8_t * block, 
                         size_t block_size,
                         rl_compress_state_t *state, 
                         timestamp_t * start_timestamp)
{
    ret_type_t status = RL_COMPRESS_SUCCESS;
    uint8_t block_num = BLOCK_NUM_INVALID;
    uint32_t compess_len = (block_size > RL_COMPRESS_SIZE_LIMIT) ? 
                              (block_size - RL_COMPRESS_SIZE_LIMIT) : block_size;
    
    if ((NULL == data) ||
        (NULL == block) ||
        (NULL == state) ||
        (NULL == start_timestamp))
    {
      status = RL_COMPRESS_ERROR_NULL;
    }
    else 
    {
      if ((RL_COMPRESS_BLOCK_SIZE_MAX < block_size) ||
         (RL_COMPRESS_BLOCK_SIZE_MIN > block_size)) 
      {
            status = RL_COMPRESS_ERROR_INVALID_PARAM;
      }
      else
      {
        block_num = get_block_num(block,block_size);
        if (block_num != BLOCK_NUM_INVALID)
        {
          rl_data_t uncomressed_block[RL_COMPRESS_BLOCK_SIZE_MAX/RL_COMPRESS_DATA_SIZE];
          if (0 != blocks_compressed_size[block_num])
          {
            if (LZF_NO_RESULT == lzf_decompress((u8 *)block, blocks_compressed_size[block_num],
                                                (u8 *)uncomressed_block, (blocks_size[block_num])))
            {
                status = RL_COMPRESS_ERROR_INVALID_STATE;
            }
          }
          else
          {
              memcpy(&uncomressed_block,
                      block, 
                      blocks_size[block_num]);
          }
          if (status == RL_COMPRESS_SUCCESS)
          {
            status = find_data_and_remove(block_num, data, uncomressed_block, 
                                          start_timestamp);
            if (status == RL_COMPRESS_SUCCESS)
            {
              memcpy(block,
                     &uncomressed_block, 
                     blocks_size[block_num]);
              blocks_compressed_size[block_num] = 0;
              blocks_payload_counter[block_num]--;
              if (0 == blocks_payload_counter[block_num])
              {
                blocks_size[block_num] = 0;
                blocks_address[block_num] = NULL;
              }
            }
          }
        }
        else
        {
           status = RL_COMPRESS_ERROR_INVALID_STATE;
        }
      }
    }
    return status;
}