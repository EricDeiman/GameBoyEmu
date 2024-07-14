#ifndef __bus_h__
#define __bus_h__

#include <string>

#include "common.hh"
#include "ram.hh"

class Bus {
public:

  Bus( RAM* );

  u8 read( u16 );
  void write( u16, u8 );

  std::string hexDump( u16, u16 );

private:
  RAM* ram;
};

#endif
