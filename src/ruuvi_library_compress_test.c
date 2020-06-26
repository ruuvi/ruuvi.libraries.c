#include "ruuvi_library.h"
#include "ruuvi_library_compress_test.h"
#include "ruuvi_library_compress.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define RL_COMPRESS_TEST_TIME_DEFAULT         16883548
#define RL_COMPRESS_TEST_FIND_TIME_DEFAULT    16883550
#define RL_COMPRESS_TEST_FIND_TIME_LOWEST     0
#define RL_COMPRESS_TEST_TEMP_DEFAULT         6.25f
#define RL_COMPRESS_TEST_HUM_DEFAULT          0.1f
#define RL_COMPRESS_TEST_PRESSURE_DEFAULT     1000.0f

#define RL_COMPRESS_TEST_TEMP_NUM             0
#define RL_COMPRESS_TEST_HUM_NUM              1
#define RL_COMPRESS_TEST_PRESSURE_NUM         2

#define RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT   64
#define RL_COMPRESS_TEST_BLOCK_SIZE_RATIO     1024
#define RL_COMPRESS_TEST_BLOCK_INVALID_MAX    (RL_COMPRESS_OVER_BLOCK_SIZE_MAX + 1)
#define RL_COMPRESS_TEST_BLOCK_INVALID_MIN    1
#define RL_COMPRESS_TEST_BLOCK_INVALID_NUM    17
#define RL_COMPRESS_TEST_DATA_SIZE_DEFAULT    RL_COMPRESS_DATA_SIZE

static rl_data_t test_data =
{
    .time = RL_COMPRESS_TEST_TIME_DEFAULT,
    .payload[RL_COMPRESS_TEST_TEMP_NUM] =
    RL_COMPRESS_TEST_TEMP_DEFAULT,
    .payload[RL_COMPRESS_TEST_HUM_NUM] =
    RL_COMPRESS_TEST_HUM_DEFAULT,
    .payload[RL_COMPRESS_TEST_PRESSURE_NUM] =
    RL_COMPRESS_TEST_PRESSURE_DEFAULT,
};

static rl_data_t find_data =
{
    .time = RL_COMPRESS_TEST_FIND_TIME_DEFAULT,
    .payload[RL_COMPRESS_TEST_TEMP_NUM] =
    RL_COMPRESS_TEST_TEMP_DEFAULT,
    .payload[RL_COMPRESS_TEST_HUM_NUM] =
    RL_COMPRESS_TEST_HUM_DEFAULT,
    .payload[RL_COMPRESS_TEST_PRESSURE_NUM] =
    RL_COMPRESS_TEST_PRESSURE_DEFAULT,
};

static rl_compress_state_t htab;

static ret_type_t ruuvi_library_test_decompress (rl_data_t * p_find_data,
        uint8_t * p_block,
        timestamp_t * p_start_timestamp)
{
    ret_type_t result;
    rl_data_t decompress_result;
    memset (&decompress_result, 0, RL_COMPRESS_TEST_DATA_SIZE_DEFAULT);
    result = rl_decompress ( (rl_data_t *) &decompress_result,
                             p_block, RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
                             &htab, p_start_timestamp);

    if (RL_COMPRESS_SUCCESS == result)
    {
#ifndef RL_COMPRESS_CONVERT_TO_INT

        if (0 != memcmp (p_find_data,
                         &decompress_result,
                         RL_COMPRESS_TEST_DATA_SIZE_DEFAULT))
        {
            result = RL_COMPRESS_ERROR_INVALID_STATE;
        }

#else

        if (p_find_data->time != decompress_result.time)
        {
            result = RL_COMPRESS_ERROR_INVALID_STATE;
        }

#endif
        memset (&decompress_result, 0, RL_COMPRESS_TEST_DATA_SIZE_DEFAULT);
    }

    return result;
}

static bool ruuvi_library_test_decompress_all (uint8_t * p_block,
        timestamp_t * p_start_timestamp,
        uint16_t block_size)
{
    bool result;
    ret_type_t res;
    find_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;

    while (res != RL_COMPRESS_END)
    {
        res = ruuvi_library_test_decompress (&find_data, p_block, p_start_timestamp);

        if (RL_COMPRESS_END == res)
        {
            result = true;
        }
        else if (RL_COMPRESS_SUCCESS == res)
        {
            find_data.time++;
        }
        else
        {
            result = false;
            break;
        }
    }

    return result;
}

static bool ruuvi_library_test_compress (uint16_t start_offset,
        rl_data_t * p_test_data,
        uint8_t * p_block)
{
    bool result =  true;
    rl_data_t decompress_result;

    for (uint16_t i = start_offset; i < RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT;
            i += RL_COMPRESS_TEST_DATA_SIZE_DEFAULT)
    {
        if (RL_COMPRESS_SUCCESS == rl_compress (p_test_data, p_block,
                                                RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
        {
            p_test_data->time++;
        }
        else
        {
            result = false;
            break;
        }
    }

    return result;
}

static bool ruuvi_library_test_invalid_state()
{
    bool result =  true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT + RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT]
        = {0};
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    rl_data_t decompress_result;
    memset (htab, 0, sizeof (htab));

    if (RL_COMPRESS_SUCCESS == rl_compress (&test_data, &block[0],
                                            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
    {
        if (RL_COMPRESS_ERROR_INVALID_STATE != rl_compress (&test_data,
                &block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT],
                RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }

    if (true == result)
    {
        if (RL_COMPRESS_ERROR_INVALID_STATE != rl_decompress ( (rl_data_t *) &decompress_result,
                &block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT],
                RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
                &htab, &start_timestamp))
        {
            result = false;
        }
        else
        {
            start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;

            if (false == ruuvi_library_test_decompress_all (&block[0], &start_timestamp,
                    RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT))
            {
                result = false;
            }
        }
    }

    test_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;
    find_data.time = RL_COMPRESS_TEST_FIND_TIME_DEFAULT;
    return result;
}

static bool ruuvi_library_test_invalid_param()
{
    bool result = true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT];
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;

    if (RL_COMPRESS_ERROR_INVALID_PARAM != rl_compress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_INVALID_MAX, &htab))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_INVALID_PARAM != rl_compress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_INVALID_MIN, &htab))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_INVALID_PARAM != rl_decompress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_INVALID_MAX,
            &htab, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_INVALID_PARAM != rl_decompress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_INVALID_MIN,
            &htab, &start_timestamp))
    {
        result = false;
    }

    return result;
}

static bool ruuvi_library_test_null()
{
    bool result = true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT];
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    size_t compessed_size;

    if (RL_COMPRESS_ERROR_NULL != rl_compress (NULL, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_compress (&test_data, NULL,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_compress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, NULL))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (NULL, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            &htab, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (&find_data, NULL,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            &htab, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            NULL, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            &htab, NULL))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_get_compressed_size (NULL,
            (size_t) RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            &compessed_size))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_get_compressed_size ( (uint8_t *) &block,
            (size_t) RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            NULL))
    {
        result = false;
    }

    return result;
}

bool ruuvi_library_test_compress_decompress()
{
    bool result =  true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT] = {0};
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    memset (htab, 0, sizeof (htab));

    if (false == ruuvi_library_test_compress (0, &test_data, &block[0]))
    {
        result = false;
    }
    else
    {
        if (false == ruuvi_library_test_decompress_all (&block[0], &start_timestamp,
                RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT))
        {
            result = false;
        }
    }

    test_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;
    find_data.time = RL_COMPRESS_TEST_FIND_TIME_DEFAULT;
    return result;
}

bool ruuvi_library_test_compress_decompress_2_times()
{
    bool result =  true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT] = { 0 };
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_DEFAULT;
    memset (htab, 0, sizeof (htab));

    if (false == ruuvi_library_test_compress (0, &test_data, &block[0]))
    {
        result = false;
    }
    else
    {
        if (RL_COMPRESS_SUCCESS != ruuvi_library_test_decompress (&find_data, &block[0],
                &start_timestamp))
        {
            result = false;
        }
        else
        {
            find_data.time = start_timestamp;

            if (false == ruuvi_library_test_compress ( (RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT -
                    RL_COMPRESS_TEST_DATA_SIZE_DEFAULT),
                    &find_data, &block[0]))
            {
                result = false;
            }
            else
            {
                test_data.time = start_timestamp;

                if (RL_COMPRESS_SUCCESS != ruuvi_library_test_decompress (&test_data, &block[0],
                        &start_timestamp))
                {
                    result = false;
                }

                start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;

                if (false == ruuvi_library_test_decompress_all (&block[0], &start_timestamp,
                        RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT))
                {
                    result = false;
                }
            }
        }
    }

    test_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;
    find_data.time = RL_COMPRESS_TEST_FIND_TIME_DEFAULT;
    return result;
}

bool ruuvi_library_test_compress_decompress_ratio (const ruuvi_library_test_print_fp
        printfp)
{
    bool result =  true;
    uint16_t counter = 0; //!< Number of bytes compressed
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_RATIO * RL_COMPRESS_TEST_DATA_SIZE_DEFAULT] = {0};
    uint16_t subblock_size = RL_COMPRESS_TEST_BLOCK_SIZE_RATIO *
                             RL_COMPRESS_TEST_DATA_SIZE_DEFAULT; //!< Size of next subblock.
    uint8_t * p_block = (uint8_t *) &block[0];
    size_t c_size = 0;
    float ratio = 0;
    ret_type_t lib_status = RL_COMPRESS_SUCCESS;
    timestamp_t start_timestamp = RL_COMPRESS_TEST_TIME_DEFAULT;
    memset (htab, 0, sizeof (htab));

    // Compress as much as possible
    while ( (RL_COMPRESS_SUCCESS == lib_status))
    {
        // Try to append uncompressed data to block.
        // rl_compress returns RL_COMPRESS_SUCCESS if data was appended
        lib_status = rl_compress (&test_data, p_block, subblock_size, &htab);

        if (RL_COMPRESS_SUCCESS == lib_status)
        {
            // Simulate change in sensor data.
            test_data.time++;
            counter += sizeof (test_data);
        }
        // Compress lib returns other status code if no new uncompressed data can
        // be appended and data was compressed in place instead.
        else if (lib_status == RL_COMPRESS_END)
        {
            c_size = 0;

            // If no new data could be added to block, the block is compressed.
            // We can reuse the space in block which still holds uncompressed data.
            if (RL_COMPRESS_SUCCESS == rl_get_compressed_size (block,
                    (size_t) subblock_size,
                    &c_size))
            {
                // Advance to start of uncompressed data.
                p_block = (uint8_t *) (&block[0] + c_size);
                // Check if there is space for a new subblock.
                subblock_size = (uint16_t) (block + sizeof (block) - p_block);

                if (RL_COMPRESS_TEST_BLOCK_SIZE_RATIO < subblock_size)
                {
                    subblock_size = RL_COMPRESS_TEST_BLOCK_SIZE_RATIO;
                    lib_status = RL_COMPRESS_SUCCESS;
                }
            }
            // If the block used for compression is corrupted, fail the test.
            else
            {
                result = false;
                break;
            }
        }
        // If compression failed while space was remaining, stop test as a failure.
        else
        {
            result = false;
            break;
        }
    }

    if (true == result)
    {
        p_block = (uint8_t *) &block[0];
        start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
        c_size = 0;

        if (RL_COMPRESS_SUCCESS == rl_get_compressed_size (p_block,
                (size_t) subblock_size,
                &c_size))
        {
            ratio = (float) c_size / (float) counter;
            char msg[128] = {0};
            snprintf (msg, sizeof (msg), "\"compress_ratio:\"\"%02.02f\%\",\r\n", ratio);
            printfp (msg);
        }

        if (false == ruuvi_library_test_decompress_all (p_block, &start_timestamp,
                (RL_COMPRESS_TEST_BLOCK_SIZE_RATIO * RL_COMPRESS_TEST_DATA_SIZE_DEFAULT)))
        {
            result = false;
        }
    }

    test_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;
    find_data.time = RL_COMPRESS_TEST_FIND_TIME_DEFAULT;
    return result;
}

bool ruuvi_library_test_invalid_input()
{
    bool result = true;

    if (false == ruuvi_library_test_null())
    {
        result = false;
    }

    if (false == ruuvi_library_test_invalid_param())
    {
        result = false;
    }

    if (false == ruuvi_library_test_invalid_state())
    {
        result = false;
    }

    return result;
}