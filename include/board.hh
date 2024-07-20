#ifndef __board_hh__
#define __board_hh_

#include <memory>

#include "common.hh"

#include "bus.hh"
#include "cpu.hh"
#include "ram.hh"
#include "serial.hh"
#include "timer.hh"

class Board {
public:
  Board();

  u8 read8( u16 );
  void _clock();

  CPU& getCpu();

private:
  RAM ram;
  Bus bus;  // Bus needs to know about all the other components
  CPU cpu;  // CPU needs to know abou the bus only
  Timer timer;  // Timer needs to know about RAM and CPU
  Serial serial;
};

#endif
