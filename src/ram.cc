
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

#include "../include/ram.hh"
#include "../include/bus.hh"

// Game Boy memory map
//  $FFFF 	      Interrupt Enable Flag
//  $FF80-$FFFE 	Zero Page - 127 bytes
//  $FF00-$FF7F 	Hardware I/O Registers
//  $FEA0-$FEFF 	Unusable Memory
//  $FE00-$FE9F 	OAM - Object Attribute Memory
//  $E000-$FDFF 	Echo RAM (duplicates $C000-$DDFF) - Reserved, Do Not Use
// 	$D000-$DFFF 	Internal RAM - Bank 1-7 (switchable - CGB only)
// 	$C000-$CFFF 	Internal RAM - Bank 0 (fixed)
// 	$A000-$BFFF 	Cartridge RAM (If Available)
// 	$9C00-$9FFF 	BG Map Data 2
// 	$9800-$9BFF 	BG Map Data 1
// 	$8000-$97FF 	Character RAM
// 	$4000-$7FFF 	Cartridge ROM - Switchable Banks 1-xx
// 	$0150-$3FFF 	Cartridge ROM - Bank 0 (fixed)
// 	$0100-$014F 	Cartridge Header Area
// 	$0000-$00FF 	Restart and Interrupt Vectors

u16
correctForEchoRAM( u16 address ) {
  if( 0xe000 <= address && address <= 0xfdff ) {
    return address & 0b1101'1111;
  }

  return address;
}

void
logUnusableRAMaccess( std::string method, u16 address ) {
  char buffer[ 1024 ] = { 0 };
  sprintf( buffer, "Attempt to %s from unusable RAM address %04x", method.c_str(),
           address );
  _log->Write( Log::warn, buffer );
  
}

void
RAM::setBus( Bus* bus ){
  _bus = bus;
}

u8
RAM::read8( u16 address ) {
  if( 0xfea0 <= address && address <= 0xfeff ) {
    logUnusableRAMaccess( "read", address );
  }

  return _ram[ correctForEchoRAM( address ) ] & 0xff;
}

void
RAM::write( u16 address, u8 data ) {
  if( 0xfea0 <= address && address <= 0xfeff ) {
    logUnusableRAMaccess( "write", address );
  }

  _ram[ correctForEchoRAM( address ) ] = data;

  if( 0xff00 <= address && address <= 0xff7f ) {
    _bus->doIO( address );
  }
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
  unsigned startAddress = start & 0xfff0;
  unsigned additional = start & 0xf;
  std::stringstream os;

  for( unsigned i = 0; i < ( count + additional ); i++ ) {
    if( i % 16 == 0 ) {
      os << std::hex << std::setfill('0') << std::setw(4)
                << i + startAddress << ": ";
    }

    if( i < additional ) {
      os << "  ";
    }
    else {
      os << std::hex << std::setfill('0') << std::setw(2)
         << ( read8( i + startAddress ) & 0xff);
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

  if( ( count + additional  ) % 16 != 0 ) {
    os << std::endl;
  }

  return os.str();
}



