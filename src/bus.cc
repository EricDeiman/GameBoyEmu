
#include "../include/bus.hh"

Bus::Bus( RAM *ram ) : ram{ ram }{}

u8
Bus::read( u16 address ) {
  return ram->read8(address);
}

std::string
Bus::hexDump( u16 start, u16 count ) {
  return ram->hexDump( start, count );
}

void
Bus::write(u16 address, u8 data ){
  ram->write( address, data );
}
