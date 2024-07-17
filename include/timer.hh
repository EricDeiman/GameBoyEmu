#ifndef __timer_hh__
#define __timer_hh__

#include "common.hh"

class CPU;
class RAM;
class Bus;

class Timer{
public:

  void initialize( CPU*, RAM*, Bus* );
  void _clock();
  void setTAC( u8 );

  enum ClockSelect {
    mcycle256 = 0,
    mcycle4   = 1,
    mcycle16  = 2,
    mcycle64  = 3
  };

private:
  std::uint64_t t_ticks = 0;
  std::uint64_t m_ticks = 0;
  CPU* cpu;
  RAM* ram;
  Bus* bus;
  bool enabled = true;
  int timerIncrement = 256;

  int increments[ 4 ] = { 256, 4, 16, 64 };
};

#endif
