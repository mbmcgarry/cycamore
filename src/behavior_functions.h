#ifndef CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_
#define CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_

// returns true every X interval (ie every 5th timestep)
bool EveryXTimestep(int curr_time, int interval);

// randomly returns true with a frequency X
// (ie returns true ~20 randomly selected timesteps
// out of 100 when frequency = 5 )
//bool EveryRandomXTimestep(int frequency);

bool EveryRandomXTimestep(int frequency, int rng_seed);

// returns a randomly generated number from a
// normal distribution defined by mean and
// sigma (full-width-half-max)
//double RNG_NormalDist(double mean, double sigma);

double RNG_NormalDist(double mean, double sigma, int rng_seed);

// returns a randomly chosen discrete number between min and max
// (ie. integer betweeen 1 and 5)

double RNG_Integer(double min, double max, int rng_seed);


#endif  //  CYCAMORE_SRC_BEHAVIOR_FUNCTIONS_H_
