#ifndef __mbc1_hh__
#define __mbc1_hh__

#include "mbc.hh"

class MBC1 : public MBC {
public:
  u16 getCartAddress( u16 );
  void write( u16, u8 );

private:
  int bank = 1;
};

#endif
