#ifndef __no_mbc_hh__
#define __no_mbc_hh__

#include "mbc.hh"

class NoMBC : public MBC {
public:
  u16 getCartAddress( u16 );
  void write( u16, u8 );
};

#endif
