#ifndef CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_
#define CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_

// returns true every X interval (ie every 5th timestep)
bool EveryXTimestep(int curr_time, int interval);

bool EveryRandomXTimestep(int frequency, int rng_seed);

double RNG_NormalDist(double mean, double sigma, int rng_seed);

#endif  //  CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_
