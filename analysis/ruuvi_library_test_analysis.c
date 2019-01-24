#include "analysis/ruuvi_library_test_analysis.h"
#include "analysis/ruuvi_library_peak2peak.h"
#include "ruuvi_library_test.h"
#include <math.h>

static size_t m_ruuvi_library_test_run;
static size_t m_ruuvi_library_test_pass;

bool ruuvi_library_test_peak2peak_ok(void)
{
  float vector[] = RLT_P2P_OK_VECTOR;
  float p2p = ruuvi_library_peak2peak(vector, sizeof(vector ) / sizeof(float));
  return ruuvi_library_expect_close(RLT_P2P_OK_EXPECT, RLT_P2P_OK_DECIMALS, p2p);
}

bool ruuvi_library_test_peak2peak_nan(void)
{
  float vector[] = RLT_P2P_NAN_VECTOR;
  float p2p = ruuvi_library_peak2peak(vector, sizeof(vector ) / sizeof(float));
  return isnan(p2p);
}

bool ruuvi_library_test_peak2peak_overflow(void)
{
  float vector[] = RLT_P2P_OVERFLOW_VECTOR;
  float p2p = ruuvi_library_peak2peak(vector, sizeof(vector ) / sizeof(float));
  return isnan(p2p);
}

bool ruuvi_library_test_peak2peak_input_check(void)
{
  float vector[] = RLT_P2P_OK_VECTOR;
  float t1, t2;
  t1 = ruuvi_library_peak2peak(NULL, 5);
  t2 = ruuvi_library_peak2peak(vector, 0);
  return isnan(t1) && isnan(t2);
}