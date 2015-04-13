#include "behavior_functions.h"
#include <ctime> // to make truly random
#include <cstdlib>
#include <iostream>
#include <cmath>

bool seeded;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryXTimestep(int curr_time, int interval) {
  // true when there is no remainder, so it is the Xth timestep
  return curr_time % interval == 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryRandomXTimestep(int frequency, bool fixed_seed) {
  if (!seeded) {
    if (fixed_seed) {
      seed_val = 1 ;
    } else {
      seed_val = time(0) ;
    }
    srand(seed_val);  //use current time as seed for RNG
    seeded = true;
  }
  int midpoint = frequency / 2;  
 
  // The interwebs say that rand is not truly random.
  //  tRan = rand() % frequency;
  int tRan = 1 + (rand()*(1.0/(RAND_MAX+1.0))) * frequency;
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

double RNG_NormalDist(double mean, double sigma, bool fixed_seed) {

  static double n2 = 0.0;
  static int n2_cached = 0;

  double result ;
  double x, y, r;

  if (!seeded) {
    if (fixed_seed) {
      seed_val = 1 ;
    } else {
      seed_val = time(0) ;
    }
    srand(seed_val);  //use current time as seed for RNG
    seeded = true;
  }

  do {
    x = 2.0*rand()/RAND_MAX - 1;
    y = 2.0*rand()/RAND_MAX - 1;
    r = x*x + y*y;
  } while (r == 0.0 || r > 1.0);
  
  double d = std::sqrt(-2.0*log(r)/r);
  double n1 = x*d;
  n2 = y*d;
  
  if (!n2_cached) {
    n2_cached = 1;
    return n1*sigma + mean;
  }
  else {
    n2_cached = 0 ;
    return n2*sigma + mean;
  }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
// Internal function to make a better random number
double uniform_deviate_ ( int seed ){
  return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
}
*/