#include "behavior_functions.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryXTimestep(int curr_time, int interval) {
  return curr_time % interval != 0;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
