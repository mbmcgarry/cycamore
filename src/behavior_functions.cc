#include "behavior_functions.h"
#include <ctime> // to make truly random
#include <cstdlib>
#include <iostream>
#include <cmath>

bool seeded;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryXTimestep(int curr_time, int interval) {
  // true when there is no remainder, so it is the Xth timestep
  if (interval <= 0) {
    return 0;
  }
  return curr_time % interval == 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryRandomXTimestep(int frequency, int rng_seed) {

  if (frequency == 0) {
    return false;
  }

  if (!seeded) {
    if (rng_seed == -1) {
      srand(time(0));    // seed random
    }
    else {
      srand(rng_seed);   // user-defined fixed seed
    }
    seeded = true;
  }
  int midpoint = frequency / 2;  
 
  // The interwebs say that rand is not truly random.
  double cur_rand = rand();
  int tRan = 1 + (cur_rand*(1.0/(RAND_MAX+1.0))) * frequency;
  //  int tRan = 1 + uniform_deviate_(rand()) * frequency;

  if (tRan == midpoint) {
    return true;
  } else {
   return false;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Use Box-Muller algorithm to make a random number sampled from
// a normal distribution

double RNG_NormalDist(double mean, double sigma, int rng_seed) {

  if (sigma <= 0 ) {
    return mean ;
  }

  static double n2 = 0.0;
  static int n2_cached = 0;

  double result ;
  double x, y, r;
  double rand1, rand2;

  if (!seeded) {
    if (rng_seed == -1) {
      srand(time(0)); // if seeding on time
    }
    else {
      srand(rng_seed);  //use fixed seed for reproducibility
    }
    seeded = true;
  }
  
  do {
    rand1 = rand();
    rand2 = rand();
    x = 2.0*rand1/RAND_MAX - 1;
    y = 2.0*rand2/RAND_MAX - 1;
    r = x*x + y*y;
  } while (r == 0.0 || r > 1.0);
  
  double d = std::sqrt(-2.0*log(r)/r);
  double n1 = x*d;
  n2 = y*d;
  
  return n1*sigma + mean;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
// Internal function to make a better random number
double uniform_deviate_ ( int seed ){
  return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
}
*/
