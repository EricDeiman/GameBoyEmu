
#include "../include/timer.hh"

void
Timer::_clock() {
  ticks++;

  if( ticks % 4 == 0 ) {
    // Do stuff
  }
}
