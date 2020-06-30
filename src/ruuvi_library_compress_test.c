#include "ruuvi_library.h"
#include "ruuvi_library_compress_test.h"
#include "ruuvi_library_compress.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define RL_COMPRESS_TEST_TIME_DEFAULT         16883548
#define RL_COMPRESS_TEST_FIND_TIME_DEFAULT    16883550
#define RL_COMPRESS_TEST_FIND_TIME_LOWEST     0
#define RL_COMPRESS_TEST_TEMP_DEFAULT         6.25f
#define RL_COMPRESS_TEST_HUM_DEFAULT          0.1f
#define RL_COMPRESS_TEST_PRESSURE_DEFAULT     1000.0f

#define RL_COMPRESS_TEST_TEMP_NUM             0
#define RL_COMPRESS_TEST_HUM_NUM              1
#define RL_COMPRESS_TEST_PRESSURE_NUM         2

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
    memset (&decompress_result, 0, sizeof(decompress_result));
    result = rl_decompress ( &decompress_result,
                             p_block, RL_COMPRESS_DECOMPRESS_SIZE,
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
        memset (&decompress_result, 0, sizeof(decompress_result));
    }

    return result;
}

static bool ruuvi_library_test_decompress_all (uint8_t * p_block,
        timestamp_t * p_start_timestamp,
        uint16_t block_size)
{
    bool result = false;
    ret_type_t res = RL_COMPRESS_SUCCESS;
    find_data.time = RL_COMPRESS_TEST_TIME_DEFAULT;
    // Clear out the decompressed data. 
    // Sanitycheck: Size of decompressed data must not have changed.
    size_t original_size = htab.decompressed_size;
    memset(htab.decompress_block, 0, RL_COMPRESS_DECOMPRESS_SIZE);
    htab.decompressed_size = 0;
    htab.compress_state = RL_COMPRESS_START;

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
    if(htab.decompressed_size != original_size)
    {
        result = false;
    }

    return result;
}

static bool ruuvi_library_test_compress (uint16_t start_offset,
        rl_data_t * p_test_data,
        uint8_t * p_block)
{
    bool result =  true;
    ret_type_t lib_status = RL_COMPRESS_SUCCESS;

    while(RL_COMPRESS_SUCCESS == lib_status)
    {
        lib_status = rl_compress (p_test_data, p_block,
                                                RL_COMPRESS_COMPRESS_SIZE, &htab);
            p_test_data->time++;
        
        // If other status code than success or done
        if(lib_status & ~(RL_COMPRESS_SUCCESS | RL_COMPRESS_END))
        {
            result = false;
        }
    }

    return result;
}

static bool ruuvi_library_test_invalid_state()
{
    bool result =  true;
    rl_data_t decompress_result;
    memset (&htab, 0, sizeof (htab));
    htab.compress_state = RL_COMPRESS_END;
    timestamp_t start_timestamp = 0;

        if (RL_COMPRESS_ERROR_INVALID_STATE != rl_decompress (&decompress_result,
                htab.compress_block,
                htab.compressed_size,
                &htab, &start_timestamp))
        {
            result = false;
        }

    return result;
}

static bool ruuvi_library_test_invalid_param()
{
    bool result = true;
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;

    // TODO

    return result;
}

static bool ruuvi_library_test_null()
{
    bool result = true;
    uint8_t block[RL_COMPRESS_COMPRESS_SIZE];
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    size_t compessed_size;

    if (RL_COMPRESS_ERROR_NULL != rl_compress (NULL, block,
            RL_COMPRESS_COMPRESS_SIZE, &htab))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_compress (&test_data, NULL,
            RL_COMPRESS_COMPRESS_SIZE, &htab))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_compress (&test_data, (uint8_t *) block,
            RL_COMPRESS_COMPRESS_SIZE, NULL))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (NULL, block,
            RL_COMPRESS_COMPRESS_SIZE,
            &htab, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (&find_data, NULL,
            RL_COMPRESS_COMPRESS_SIZE,
            &htab, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_COMPRESS_SIZE,
            NULL, &start_timestamp))
    {
        result = false;
    }

    if (RL_COMPRESS_ERROR_NULL != rl_decompress (&test_data, (uint8_t *) &block,
            RL_COMPRESS_COMPRESS_SIZE,
            &htab, NULL))
    {
        result = false;
    }

    return result;
}

bool ruuvi_library_test_compress_decompress()
{
    bool result =  true;
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    memset (&htab, 0, sizeof (htab));

    if (false == ruuvi_library_test_compress (0, &test_data, htab.compress_block))
    {
        result = false;
    }
    else
    {
        if (false == ruuvi_library_test_decompress_all (htab.compress_block, &start_timestamp,
                RL_COMPRESS_COMPRESS_SIZE))
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
    timestamp_t start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
    memset (&htab, 0, sizeof (htab));

    if (false == ruuvi_library_test_compress (0, &test_data, htab.compress_block))
    {
        result = false;
    }
    else
    {
        if (false == ruuvi_library_test_decompress_all (htab.compress_block, &start_timestamp,
                RL_COMPRESS_COMPRESS_SIZE))
        {
            result = false;
        }
        if (false == ruuvi_library_test_decompress_all (htab.compress_block, &start_timestamp,
                RL_COMPRESS_COMPRESS_SIZE))
        {
            result = false;
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
    size_t c_size = 0;
    float ratio = 0;
    // Always have same random data series
    srand(1);
    ret_type_t lib_status = RL_COMPRESS_SUCCESS;
    timestamp_t start_timestamp = RL_COMPRESS_TEST_TIME_DEFAULT;
    memset (&htab, 0, sizeof (htab));

    // Compress as much as possible
    while ( (RL_COMPRESS_SUCCESS == lib_status))
    {
        // Try to append uncompressed data to block.
        // rl_compress returns RL_COMPRESS_SUCCESS if data was appended
        lib_status = rl_compress (&test_data, htab.compress_block, RL_COMPRESS_COMPRESS_SIZE, &htab);

        if (RL_COMPRESS_SUCCESS == lib_status)
        {
            // Simulate change in sensor data.
            test_data.time++;
            test_data.payload[RL_COMPRESS_TEST_TEMP_NUM] += (float)((rand()%100)/50.0F)-0.5F;
            test_data.payload[RL_COMPRESS_TEST_HUM_NUM] += (float)((rand()%100)/50.0F)-0.5F;
            test_data.payload[RL_COMPRESS_TEST_PRESSURE_NUM] += (float)((rand()%100)/50.0F)-0.5F;
            counter += sizeof (test_data);
        }
        // Compress lib returns other status code if no new uncompressed data can
        // be appended and data was compressed in place instead.
        else if (lib_status == RL_COMPRESS_END)
        {

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
        start_timestamp = RL_COMPRESS_TEST_FIND_TIME_LOWEST;
        c_size = 0;

            ratio = (float) htab.compressed_size / (float) counter;
            char msg[128] = {0};
            snprintf (msg, sizeof (msg), "\"compress_ratio:\"\"%02.02f\%\",\r\n", ratio);
            printfp (msg);

        if (false == ruuvi_library_test_decompress_all (htab.compress_block, &start_timestamp,
                RL_COMPRESS_COMPRESS_SIZE))
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
