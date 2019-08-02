#include "ruuvi_library_pla.h"
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

bool ruuvi_library_delta_state_1ch_update(const ruuvi_library_delta_state_1ch_t* const start,
                                          ruuvi_library_delta_state_1ch_t* const now)
{
  // If minimum update interval has not been reached, do not update. 
  const uint64_t delta_t = now->timestamp - start->timestamp;
  if(delta_t < start->interval_min) { return false; }

  // if maximum time interval between updates has not been reached, check if 
  // state is still valid
  if(delta_t < start->interval_max) 
  {
    // Estimate state
    float estimate = start->value1 + start->delta1 * delta_t;

    // Compare estimated value to measured value. If we're in estimate no update is necessary.
    float epsilon = fabsf(estimate - now->value1);
    if(epsilon < start->epsilon1) { return false; }
  }

  // Our estimation has diverged from measurement. End segment, start a new segment.
  now->delta1 = (now->value1 - start->value1) / delta_t;
  return true;
}