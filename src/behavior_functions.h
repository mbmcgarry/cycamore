#ifndef CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_
#define CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_

// returns true every X interval (ie every 5th timestep)
bool EveryXTimestep(int curr_time, int interval);

// randomly returns true with a frequency X
// (ie returns true ~20 randomly selected timesteps
// out of 100 when frequency = 5 )
bool EveryRandomXTimestep(int frequency);


// returns a randomly generated number from a
// normal distribution defined by mean and
// sigma (full-width-half-max)
double RNG_NormalDist(double mean, double sigma);


#endif  //  CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_
