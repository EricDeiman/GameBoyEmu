#ifndef __serial_hh__
#define __serial_hh__

#include <fstream>

#include "common.hh"

class Bus;

struct Serial {

  Serial();
  
  void initialize( Bus* bus );

  void write();

private:
  Bus* bus;
  std::ofstream os;

};

#endif
