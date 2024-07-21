
#include "../include/mbc1.hh"

u16
MBC1::getCartAddress( u16 address ) {
  if( address < 0x4000 ) {
    return address;
  }

  u16 offset = address - 0x4000;

  return bank * 0x4000 + offset;
}

void
MBC1::write( u16 address, u8 data ) {
  data = data & 0b1'1111;
  if( data == 0 ) {
    data = 1;
  }
  
  bank = data;
}
