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
#define RL_COMPRESS_TEST_BLOCK_SIZE_RATIO     256
#define RL_COMPRESS_TEST_BLOCK_NUM_RATIO      4
#define RL_COMPRESS_TEST_BLOCK_INVALID_MAX    4097
#define RL_COMPRESS_TEST_BLOCK_INVALID_MIN    32
#define RL_COMPRESS_TEST_BLOCK_INVALID_NUM    17
#define RL_COMPRESS_TEST_DATA_SIZE_DEFAULT    sizeof (rl_data_t)

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

rl_compress_state_t htab;

static bool ruuvi_library_test_decompress (rl_data_t * p_find_data,
        uint8_t * p_block,
        timestamp_t * p_start_timestamp)
{
    bool result =  true;
    rl_data_t decompress_result;
    memset (&decompress_result, 0, RL_COMPRESS_TEST_DATA_SIZE_DEFAULT);

    if (RL_COMPRESS_SUCCESS == rl_decompress ( (rl_data_t *) &decompress_result,
            p_block, RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT,
            &htab, p_start_timestamp))
    {
#ifndef RL_COMPRESS_CONVERT_TO_INT

        if (0 != memcmp (p_find_data,
                         &decompress_result,
                         RL_COMPRESS_TEST_DATA_SIZE_DEFAULT))
        {
            result = false;
        }

#else

        if (p_find_data->time != decompress_result.time)
        {
            result = false;
        }

#endif
        memset (&decompress_result, 0, RL_COMPRESS_TEST_DATA_SIZE_DEFAULT);
    }
    else
    {
        result = false;
    }

    return result;
}

static bool ruuvi_library_test_decompress_all (uint8_t * p_block,
        timestamp_t * p_start_timestamp,
        uint16_t block_size)
{
    bool result =  true;
    find_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;

    for (uint16_t i = 0; i < block_size;
            i += RL_COMPRESS_TEST_DATA_SIZE_DEFAULT)
    {
        if (false == ruuvi_library_test_decompress (&find_data, p_block, p_start_timestamp))
        {
            result = false;
            break;
        }
        else
        {
            find_data.time++;
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
    bool result = true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT * RL_COMPRESS_TEST_BLOCK_INVALID_NUM];
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    size_t compessed_size;

    for (uint8_t i = 0; i < (RL_COMPRESS_TEST_BLOCK_INVALID_NUM - 1); i++)
    {
        test_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;

        if (false == ruuvi_library_test_compress (0, &test_data,
                (uint8_t *) ( (uint8_t *) &block +
                              (i * RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT))))
        {
            result = false;
            break;
        }
    }

    if (result ==  true)
    {
        if (RL_COMPRESS_ERROR_INVALID_STATE != rl_get_compressed_size ( (uint8_t *) ( (
                    uint8_t *) &block +
                (RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT *
                 RL_COMPRESS_TEST_BLOCK_INVALID_NUM)),
                &compessed_size))
        {
            result = false;
        }
    }

    if (result ==  true)
    {
        if (RL_COMPRESS_ERROR_INVALID_STATE != rl_compress (&test_data,
                (uint8_t *) ( (uint8_t *) &block +
                              (RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT *
                               RL_COMPRESS_TEST_BLOCK_INVALID_NUM)),
                RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
        {
            result = false;
        }
        else
        {
            if (RL_COMPRESS_ERROR_INVALID_STATE != rl_compress (&test_data, (uint8_t *) &block,
                    RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab))
            {
                result = false;
            }
            else
            {
                if (RL_COMPRESS_ERROR_INVALID_STATE != rl_decompress (&test_data,
                        (uint8_t *) ( (uint8_t *) &block +
                                      (RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT *
                                       RL_COMPRESS_TEST_BLOCK_INVALID_NUM)),
                        RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT, &htab,
                        &start_timestamp))
                {
                    result = false;
                }
                else
                {
                    for (uint8_t i = 0; i < (RL_COMPRESS_TEST_BLOCK_INVALID_NUM - 1); i++)
                    {
                        start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;

                        if (false == ruuvi_library_test_decompress_all ( (uint8_t *) ( (uint8_t *) &block +
                                (i * RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT)),
                                &start_timestamp, RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT))
                        {
                            result = false;
                            break;
                        }
                    }
                }
            }
        }
    }

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

    if (RL_COMPRESS_ERROR_NULL != rl_get_compressed_size (NULL, &compessed_size))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_get_compressed_size ( (uint8_t *) &block, NULL))
    {
        result = false;
    }

    return result;
}

bool ruuvi_library_test_compress_decompress()
{
    bool result =  true;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT];
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
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT];
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_DEFAULT;
    memset (htab, 0, sizeof (htab));

    if (false == ruuvi_library_test_compress (0, &test_data, &block[0]))
    {
        result = false;
    }
    else
    {
        if (false == ruuvi_library_test_decompress (&find_data, &block[0], &start_timestamp))
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

                if (false == ruuvi_library_test_decompress (&test_data, &block[0], &start_timestamp))
                {
                    result = false;
                }

                //Used to clear compressed block
                start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
                ruuvi_library_test_decompress_all (&block[0], &start_timestamp,
                                                   RL_COMPRESS_TEST_BLOCK_SIZE_DEFAULT);
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
    uint16_t counter = 0;
    uint8_t block[RL_COMPRESS_TEST_BLOCK_SIZE_RATIO * RL_COMPRESS_TEST_DATA_SIZE_DEFAULT];
    uint8_t * p_block = (uint8_t *) &block[0];
    uint8_t * f_block = (uint8_t *) &block[0];
    size_t c_size = 0;
    float ratio = 0;
    timestamp_t start_timestamp = RL_COMPRESS_TEST_TIME_DEFAULT;
    memset (htab, 0, sizeof (htab));

    for (counter;
            counter < (RL_COMPRESS_TEST_BLOCK_SIZE_RATIO * RL_COMPRESS_TEST_BLOCK_NUM_RATIO);
            counter += RL_COMPRESS_TEST_DATA_SIZE_DEFAULT)
    {
        if (RL_COMPRESS_SUCCESS == rl_compress (&test_data, p_block,
                                                RL_COMPRESS_TEST_BLOCK_SIZE_RATIO, &htab))
        {
            test_data.time++;
        }
        else
        {
            c_size = 0;

            if (RL_COMPRESS_SUCCESS == rl_get_compressed_size (p_block, &c_size))
            {
                test_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;
                p_block += RL_COMPRESS_TEST_BLOCK_SIZE_RATIO;
                counter -= RL_COMPRESS_TEST_DATA_SIZE_DEFAULT;
            }
            else
            {
                result = false;
                break;
            }
        }
    }

    if (true == result)
    {
        for (counter = 0; counter < RL_COMPRESS_TEST_BLOCK_NUM_RATIO ; counter++)
        {
            start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;

            if (RL_COMPRESS_SUCCESS == rl_get_compressed_size (p_block, &c_size))
            {
                ratio = (float) (RL_COMPRESS_TEST_BLOCK_SIZE_RATIO);
                ratio = (float) (ratio / c_size);
                char msg[128] = {0};
                snprintf (msg, sizeof (msg), "\"block_address %d, compress_ratio %02.02f\",\r\n",
                          p_block, ratio);
                printfp (msg);
            }

            if (false == ruuvi_library_test_decompress_all (p_block, &start_timestamp,
                    RL_COMPRESS_TEST_BLOCK_SIZE_RATIO))
            {
                result = false;
                break;
            }

            p_block -= RL_COMPRESS_TEST_BLOCK_SIZE_RATIO;
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