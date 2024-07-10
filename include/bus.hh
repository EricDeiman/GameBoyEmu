#ifndef __bus_h__
#define __bus_h__

#include "common.hh"
#include "ram.hh"

class Bus {
public:

  Bus( RAM* );

  u8 read( u16 );
  void write( u16, u8 );

private:
  RAM* ram;
};

#endif
