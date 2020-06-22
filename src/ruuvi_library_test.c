#include "ruuvi_library_enabled_modules.h"
#if RL_INTEGRATION_TEST_ENABLED
#include "ruuvi_library_test.h"
#include "ruuvi_library_test_analysis.h"
#include "ruuvi_library_ringbuffer_test.h"
#include "ruuvi_library_compress_test.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Run peak to peak tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_peak2peak(uint32_t* const total_tests, uint32_t* const passed, const ruuvi_library_test_print_fp printfp)
{
  bool pass = false;

  printfp("'peak2peak':{\r\n");
  printfp("'valid_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_peak2peak_ok();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  printfp("'nan_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_peak2peak_nan();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  printfp("'overflow_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_peak2peak_overflow();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  printfp("'input_validation':");
  (*total_tests)++;
  pass = ruuvi_library_test_peak2peak_input_check();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");
  printfp("}");
}

/**
 * @brief Run root mean square tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_rms(uint32_t* const total_tests, uint32_t* const passed, const ruuvi_library_test_print_fp printfp)
{
  bool pass = false;
  printfp("'rms':{\r\n");
  printfp("'valid_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_rms_ok();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  (*total_tests)++;
  printfp("'nan_data':");
  pass = ruuvi_library_test_rms_nan();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  (*total_tests)++;
  printfp("'overflow_data':");
  pass = ruuvi_library_test_rms_overflow();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  (*total_tests)++;
  printfp("'input_validation':");
  pass = ruuvi_library_test_rms_input_check();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");
  printfp("}");
}

/**
 * @brief Run variance tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_variance(uint32_t* const total_tests, uint32_t* const passed, const ruuvi_library_test_print_fp printfp)
{
  bool pass = false;
  printfp("'variance':{\r\n");
  printfp("'valid_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_variance_ok();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  printfp("'nan_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_variance_nan();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  printfp("'overflow_data':");
  (*total_tests)++;
  pass = ruuvi_library_test_variance_overflow();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");

  printfp("'input_validation':");
  (*total_tests)++;
  pass = ruuvi_library_test_variance_input_check();
  (*passed) += pass;
  pass ? printfp("'pass',\r\n") : printfp("'fail',\r\n");
  printfp("}");
}

/**
 * @brief Run Ringbuffer tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_ringbuffer(uint32_t* const total_tests, uint32_t* const passed, const ruuvi_library_test_print_fp printfp)
{
  (*total_tests)++;
  (*passed) += ruuvi_library_test_ringbuffer_put_get();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_ringbuffer_overflow();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_ringbuffer_underflow();
}

/**
 * @brief Run compress tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_compress(uint32_t* const total_tests, uint32_t* const passed, const ruuvi_library_test_print_fp printfp)
{
  (*total_tests)++;
  (*passed) += ruuvi_library_test_compress_decompress();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_compress_decompress_2_times();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_invalid_input();
}
  
bool ruuvi_library_test_all_run(const ruuvi_library_test_print_fp printfp)
{ 
  uint32_t total_tests = 0;
  uint32_t passed = 0;
  ruuvi_library_test_peak2peak(&total_tests, &passed, printfp);
  printfp(",\r\n");
  ruuvi_library_test_rms(&total_tests, &passed, printfp);
  printfp(",\r\n");
  ruuvi_library_test_variance(&total_tests, &passed, printfp);
  printfp(",\r\n");
  ruuvi_library_test_ringbuffer(&total_tests, &passed, printfp);
  printfp(",\r\n");
  ruuvi_library_test_compress(&total_tests, &passed, printfp);

  char msg[128] = {0};
  snprintf(msg, sizeof(msg), "'total_tests':'%lu',\r\n'passed_tests':'%lu'\r\n", total_tests, passed);
  printfp(msg);

  return (total_tests == passed);
}

bool ruuvi_library_expect_close(const float expect, const int8_t precision, const float check)
{
  if(!isfinite(expect) || !isfinite(check)) { return false; }
  const float max_delta = pow(10, -precision);
  float delta = fabs(expect - check);
  return max_delta > delta;
}
#endif