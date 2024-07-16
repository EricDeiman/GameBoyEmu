
#include "../include/board.hh"

Board::Board() {
  ram.setBus( &bus );
}

const CPU&
Board::getCpu() {
  return cpu;
}

u8
Board::read8( u16 address ) {
  return ram.read8(address);
}

void
Board::_clock() {
  timer._clock();
  cpu._clock();
}
