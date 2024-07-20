
#include <algorithm>
#include <filesystem>
#include <iostream>

#include "../include/serial.hh"

#include "../include/bus.hh"

Serial::Serial() {
  auto keys = conf->GetKeys();
  auto hasSerialLog = std::find( keys.begin(), keys.end(), "SerialLog" );

  if( hasSerialLog != keys.end() ) {
    os.open( *hasSerialLog, std::ios::app );
  }
}

void
Serial::initialize( Bus *bus ) {
  this->bus = bus;
}

void
Serial::write() {
  if( os.is_open() ) {
    auto control = (bus->read(Bus::IOAddress::SC)) & 0xff;
    if ((control & 0x81) == 0x81) {
      u8 data = (bus->read(Bus::IOAddress::SB)) & 0xff;
      os << static_cast<char>(data);
    }
  }
}
