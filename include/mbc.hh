#ifndef __mbc_hh__
#define __mbc_hh__

#include "common.hh"

class MBC {
public:
  virtual u16 getCartAddress( u16 ) = 0;
  virtual void write( u16, u8 ) = 0;
};

#endif
