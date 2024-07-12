
#include "../include/board.hh"

u8
Board::read8( u16 address ) {
  return ram.read8(address);
}

void
Board::_clock() {
  cpu._clock();
}
