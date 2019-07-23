#include "ruuvi_library_test.h"
#include "analysis/ruuvi_library_test_analysis.h"
#include "data_structures/ruuvi_library_ringbuffer_test.h"
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
static void ruuvi_library_test_peak2peak(uint32_t* const total_tests, uint32_t* const passed)
{
  (*total_tests)++;
  (*passed) += ruuvi_library_test_peak2peak_ok();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_peak2peak_nan();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_peak2peak_overflow();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_peak2peak_input_check();
}

/**
 * @brief Run root mean square tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_rms(uint32_t* const total_tests, uint32_t* const passed)
{
  (*total_tests)++;
  (*passed) += ruuvi_library_test_rms_ok();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_rms_nan();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_rms_overflow();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_rms_input_check();
}

/**
 * @brief Run variance tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_variance(uint32_t* const total_tests, uint32_t* const passed)
{
  (*total_tests)++;
  (*passed) += ruuvi_library_test_variance_ok();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_variance_nan();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_variance_overflow();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_variance_input_check();
}

/**
 * @brief Run Ringbuffer tests
 *
 * @param[in, out] total Total number of tests which have been run. Gets incremented.
 * @param[in, out] passed Total number of tests which have passed. Gets incremented.
 */
static void ruuvi_library_test_ringbuffer(uint32_t* const total_tests, uint32_t* const passed)
{
  (*total_tests)++;
  (*passed) += ruuvi_library_test_ringbuffer_put_get();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_ringbuffer_overflow();

  (*total_tests)++;
  (*passed) += ruuvi_library_test_ringbuffer_underflow();
}
  
bool ruuvi_library_test_all_run(const ruuvi_library_test_print_fp printfp)
{ 
  uint32_t total_tests = 0;
  uint32_t passed = 0;

  ruuvi_library_test_peak2peak(&total_tests, &passed);
  ruuvi_library_test_rms(&total_tests, &passed);
  ruuvi_library_test_variance(&total_tests, &passed);
  ruuvi_library_test_ringbuffer(&total_tests, &passed);

  char msg[128] = {0};
  snprintf(msg, sizeof(msg), "Library tests ran: %lu, passed: %lu.\r\n", total_tests, passed);
  printfp(msg);

  return (total_tests == passed);
}

bool ruuvi_library_expect_close(const float expect, const int8_t precision, const float check)
{
  if(!isfinite(expect) || !isfinite(check)) { return false; }
  const float max_delta = pow(10, precision);
  float delta = expect - check;
  if(delta < 0) { delta = 0 - delta; } // absolute value
  return max_delta > delta;
}