
#include "../include/board.hh"

Board::Board() {
  bus.initialize( &cpu, &ram, &timer, &serial );
  cpu.initialize( &bus );
  timer.initialize( &cpu, &ram, &bus );
  serial.initialize( &bus );
}

u8
Board::read8( u16 address ) {
  return ram.read8(address);
}

void
Board::_clock() {
  timer._clock();
  cpu._clock();  // keep the cpu as the last call
}

CPU&
Board::getCpu() {
  return cpu;
}
