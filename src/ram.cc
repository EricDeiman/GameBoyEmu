
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

#include "../include/ram.hh"

u8
RAM::read8( u16 address ) {
  return _ram[address] & 0xff;
}

void
RAM::write( u16 address, u8 data ) {
  _ram[ address ] = data;
} 

RAM::RAM() {
  std::string cartFileName;

  try {
    auto keys = conf->GetKeys();

    auto hasCartConfig = std::find( keys.begin(), keys.end(), "Cart" );

    if( hasCartConfig != keys.end() ) {
      cartFileName = conf->GetValue("Cart");

      fs::path fileName{ cartFileName };

      auto fileSize{ fs::file_size( fileName ) };
      _ram.reserve( fileSize );

      std::ifstream inFile{ fileName };
      inFile.read( _ram.data(), fileSize );

      _log->Write( Log::info, "Finished loading cartridge file " + cartFileName );
    }
    else {
      _log->Write(Log::error, "No cartridge file to open" );
    }
  }
  catch( std::exception& ) {
    _log->Write( Log::error, "Unable to open cartridge file " + cartFileName );
  }
}

std::string
RAM::hexDump( u16 start, u16 count ){
  auto startAddress = start & 0xff00;
  auto additional = start & 0xff;
  std::stringstream os;

  for( auto i = 0; i < ( count + additional ); i++ ) {
    if( i % 16 == 0 ) {
      os << std::hex << std::setfill('0') << std::setw(4)
                << i + startAddress << ": ";
    }

    if( i < additional ) {
      os << "  ";
    }
    else {
      os << std::hex << std::setfill('0') << std::setw(2)
                << (_ram[i + startAddress] & 0xff);
    }

    if( i % 8 == 7 ) {
      os << "  ";
    }
    else if( i % 2 == 1 ) {
      os << " ";
    }

    if( i % 16 == 15 ) {
      os << std::endl;
    }
  }

  if( ( count + additional  ) % 16 != 15 ) {
    os << std::endl;
  }

  return os.str();
}



