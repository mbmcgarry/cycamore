#include "behavior_functions.h"
#include <ctime> // to make truly random
#include <cstdlib>
#include <iostream>

bool seeded ;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryXTimestep(int curr_time, int interval) {
  //  std::cout << "for t=" << curr_time << "Result is " << (curr_time % interval == 0) << std::endl;
  // true when there is no remainder, so it is the Xth timestep
  return curr_time % interval == 0;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EveryRandomXTimestep(int frequency) {
  if (!seeded) {
    srand(time(0)) ;  //use current time as seed for RNG
    seeded = true ;
  }
  int midpoint = frequency / 2 ;  
 
  // The interwebs say that rand is not truly random.
  //  tRan = rand() % frequency ;
  int tRan = 1 + (rand()*(1.0/(RAND_MAX+1.0))) * frequency;
  //  int tRan = 1 + uniform_deviate_(rand()) * frequency;

  if (tRan == midpoint) {
    return true;
  } else {
   return false ;
  }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Internal function to make a better random number
/*
double uniform_deviate_ ( int seed ){
  return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
}
*/