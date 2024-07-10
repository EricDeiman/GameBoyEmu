#ifndef __ram_hh__
#define __ram_hh__

#include <fstream>
#include <iostream>
#include <vector>

#include "common.hh"

// TODO:This is where the address map needs to be implemented.

class RAM {
public:

  RAM();

  u8 read8( u16 );
  void write( u16, u8 );

  std::string hexDump( u16, u16 );

private:
  std::vector< char > _ram;
};

#endif
