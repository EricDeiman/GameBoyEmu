#ifndef __timer_hh__
#define __timer_hh__

#include "common.hh"

class Timer{
public:
  void _clock();

private:
  std::uint64_t ticks = 0;

};

#endif
