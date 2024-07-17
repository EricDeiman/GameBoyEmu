
#include "../include/timer.hh"
#include "../include/cpu.hh"
#include "../include/ram.hh"
#include "../include/bus.hh"

void
Timer::initialize( CPU* cpu, RAM* ram, Bus* bus ) {
  this->cpu = cpu;
  this->ram = ram;
  this->bus = bus;
}

void
Timer::_clock() {
  t_ticks++;

  if( t_ticks % 256 == 0 )  {
    // Update the divider register
    ram->write( Bus::IOAddress::DIV,
              ( bus->read( Bus::IOAddress::DIV ) + 1 ) & 0xff );
  }

  if( t_ticks % 4 == 0 ) {
    m_ticks++;

    if( enabled ) {
      if( m_ticks % timerIncrement == 0 ) {
        // Check TIMA.  If it is 0xff, reset to the value in TMA and generate intrrupt
        if( ram->read8( Bus::IOAddress::TIMA ) == 0xff ) {
          ram->write( Bus::IOAddress::TIMA,
                      ram->read8( Bus::IOAddress::TMA ) );
          cpu->triggerInterrupt( CPU::Interrupt::Timer );
        }
      }
    }
  }
}

void
Timer::setTAC( u8 data ) {
  u8 enableMask = 0x4;
  u8 clockSelectMask = 0x3;

  if( ( data & enableMask ) > 0 ) {
    enabled = true;
  }
  else {
    enabled = false;
  }

  timerIncrement = increments[ ( data & clockSelectMask ) ];
}
