#include "ruuvi_library_test.h"
#include "analysis/ruuvi_library_test_analysis.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
  
bool ruuvi_library_test_all_run(const ruuvi_library_test_print_fp printfp)
{ 
  uint32_t total_tests = 0;
  uint32_t passed = 0;

  total_tests++;
  passed += ruuvi_library_test_peak2peak_ok();

  total_tests++;
  passed += ruuvi_library_test_peak2peak_nan();

  total_tests++;
  passed += ruuvi_library_test_peak2peak_overflow();

  total_tests++;
  passed += ruuvi_library_test_peak2peak_input_check();

  total_tests++;
  passed += ruuvi_library_test_rms_ok();

  total_tests++;
  passed += ruuvi_library_test_rms_nan();

  total_tests++;
  passed += ruuvi_library_test_rms_overflow();

  total_tests++;
  passed += ruuvi_library_test_rms_input_check();

  total_tests++;
  passed += ruuvi_library_test_variance_ok();

  total_tests++;
  passed += ruuvi_library_test_variance_nan();

  total_tests++;
  passed += ruuvi_library_test_variance_overflow();

  total_tests++;
  passed += ruuvi_library_test_variance_input_check();

  char msg[128] = {0};
  snprintf(msg, sizeof(msg), "Library tests ran: %d, passed: %d.\r\n", total_tests, passed);
  printfp(msg);
}

bool ruuvi_library_expect_close(const float expect, const int8_t precision, const float check)
{
  if(!isfinite(expect) || !isfinite(check)) { return false; }
  const float max_delta = pow(10, precision);
  float delta = expect - check;
  if(delta < 0) delta = 0 - delta; // absolute value
  return max_delta > delta;
}