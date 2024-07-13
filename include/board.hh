#ifndef __board_hh__
#define __board_hh_

#include <memory>

#include "bus.hh"
#include "common.hh"
#include "cpu.hh"
#include "ram.hh"

class Board {
public:

  const CPU& getCpu();
  
  u8 read8( u16 );
  void _clock();

private:
  RAM ram;
  Bus bus{ &ram };
  CPU cpu{ &bus };
};

#endif
