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
bool EveryRandomXTimestep(int frequency, bool time_seed) {

  if (frequency == 0) {
    return false;
  }

  if (!seeded) {
    if (time_seed) {
      srand(time(0)); // if seeding on time
    }
    else {
      srand(1);  //use fixed seed for reproducibility
    }
    seeded = true;
  }
  int midpoint = frequency / 2;  
 
  // The interwebs say that rand is not truly random.
  //  tRan = rand() % frequency;
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
/*
bool EveryRandomXTimestep(int frequency) {
  bool time_seed = 0 ;
  return EveryRandomXTimestep(frequency, time_seed);
}
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Use Box-Muller algorithm to make a random number sampled from
// a normal distribution

double RNG_NormalDist(double mean, double sigma, bool time_seed) {

  if (sigma == 0 ) {
    return mean ;
  }

  static double n2 = 0.0;
  static int n2_cached = 0;

  double result ;
  double x, y, r;

  if (!seeded) {
    if (time_seed) {
      srand(time(0)); // if seeding on time
    }
    else {
      srand(1);  //use fixed seed for reproducibility
    }
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
  
  /*
  if (!n2_cached) {
    n2_cached = 1;
    return n1*sigma + mean;
  }
  else {
    n2_cached = 0 ;
    return n2*sigma + mean;
  }
  */
    std::cout << "random number is " << n1*sigma + mean << std::endl;
  return n1*sigma + mean;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
double RNG_NormalDist(double mean, double sigma) {
  bool time_seed = 0;
  return RNG_NormalDist(mean, sigma, time_seed);
}
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
// Internal function to make a better random number
double uniform_deviate_ ( int seed ){
  return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
}
*/
