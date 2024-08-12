
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

#include "../include/ram.hh"

#include "../include/bus.hh"
#include "../include/cpu.hh"
#include "../include/mbc1.hh"
#include "../include/no_mbc.hh"

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

  address = correctForEchoRAM(address);

  if( address <= maxCartRom ) {
    return _cart[ mbc->getCartAddress( address ) ] & 0xff;
  }

  return _ram[ address ] & 0xff;
}

void
RAM::write( u16 address, u8 data ) {
  if( 0xfea0 <= address && address <= 0xfeff ) {
    logUnusableRAMaccess( "write", address );
  }

  if( address <= maxCartRom ) {
    mbc->write( address, data );
  }

  _ram[ correctForEchoRAM( address ) ] = data;
}

void
RAM::dbgWrite( u16 address, u8 data ) {
  // Where to write: cart or ram?
  if( address <= maxCartRom ) {
    _cart[ address ] = data;
  }
  else {
    _ram[ address ] = data;
  }
}

dictionary< int, std::string >CartType = {
  { 0x00, "ROM ONLY" },
  { 0x01, "MBC1" },
  { 0x02, "MBC1+RAM" },
  { 0x03, "MBC1+RAM+BATTERY" },
  { 0x05, "MBC2" },
  { 0x06, "MBC2+BATTERY" },
  { 0x08, "ROM+RAM" },
  { 0x09, "ROM+RAM+BATTERY" },
  { 0x0B, "MMM01" },
  { 0x0C, "MMM01+RAM" },
  { 0x0D, "MMM01+RAM+BATTERY" },
  { 0x0F, "MBC3+TIMER+BATTERY" },
  { 0x10, "MBC3+TIMER+RAM+BATTERY" },
  { 0x11, "MBC3" },
  { 0x12, "MBC3+RAM" },
  { 0x13, "MBC3+RAM+BATTERY" },
  { 0x19, "MBC5" },
  { 0x1A, "MBC5+RAM" },
  { 0x1B, "MBC5+RAM+BATTERY" },
  { 0x1C, "MBC5+RUMBLE" },
  { 0x1D, "MBC5+RUMBLE+RAM" },
  { 0x1E, "MBC5+RUMBLE+RAM+BATTERY" },
  { 0x20, "MBC6" },
  { 0x22, "MBC7+SENSOR+RUMBLE+RAM+BATTERY" },
  { 0xFC, "POCKET CAMERA" },
  { 0xFD, "BANDAI TAMA5" },
  { 0xFE, "HuC3" },
  { 0xFF, "HuC1+RAM+BATTERY" },
};

std::string ROMsizes[] = {
  "32 KiB",
  "64  KiB",
  "128 KiB",
  "256 KiB",
  "512 KiB",
  "1 MiB",
  "2 MiB",
  "4 MiB",
  "8 MiB",
};

std::string RAMsizes[] = {
    "No RAM",
    "Public domain cartridge",
    "8 KiB",
    "32 KiB",
    "128 KiB",
    "64 KiB"
};

RAM::RAM() {
  std::string cartFileName;

  try {
    _ram.reserve( 0xffff );

    // TODO: debugging with gameboy doctor, should be removed at some point
    _ram[ 0xff44 ] = 0x90;

    auto keys = conf->GetKeys();

    auto hasCartConfig = std::find( keys.begin(), keys.end(), "Cart" );

    if( hasCartConfig != keys.end() ) {
      cartFileName = conf->GetValue("Cart");

      fs::path fileName{ cartFileName };

      auto fileSize{ fs::file_size( fileName ) };
      _cart.reserve( fileSize );

      std::ifstream inFile{ fileName };
      inFile.read( _cart.data(), fileSize );

      _log->Write( Log::info, "Finished loading cartridge file " + cartFileName );

      // Print cart header to log
      char buffer[ 1024 ] = { 0 };
      sprintf( buffer, "   Title = %s", static_cast< char* >( _cart.data() + 0x134 ) );
      _log->Write( Log::info, buffer );

      sprintf( buffer, "   Old licensee code = 0x%02x", _cart[ 0x14b ] );
      _log->Write( Log::info, buffer );

      sprintf( buffer, "   New licensee code = %c%c",
               static_cast< char >( _cart[ 0x144 ] ),
               static_cast< char >( _cart[ 0x145 ] ) );
      _log->Write( Log::info, buffer );

      sprintf( buffer, "   Cartridge type = 0x%02x ( %s )",
               _ram[ 0x147 ], CartType[ _cart[ 0x147 ] ].c_str() );
      _log->Write( Log::info, buffer );
      switch( _cart[ 0x147 ] ) {

      case 0x0:
        mbc = std::make_shared< NoMBC >();
        break;

      case 0x1:
        mbc = std::make_shared< MBC1 >();
        break;

      default: {
        char buffer[ 1024 ] = { 0 };
        sprintf( buffer, "Unknown cartridge type 0x%02x", _cart[ 0x147 ] );
        throw std::runtime_error( buffer );
        }
        break;
      }

      sprintf( buffer, "   ROM size = 0x%02x ( %s )",
               _ram[ 0x148 ], ROMsizes[ static_cast< int >( _cart[ 0x148 ] ) ].c_str() );
      _log->Write( Log::info, buffer );

      sprintf( buffer, "   RAM size = 0x%02x ( %s )",
               _ram[ 0x149 ], RAMsizes[ static_cast< int >( _cart[ 0x149 ] ) ].c_str() );
      _log->Write( Log::info, buffer );

      sprintf( buffer, "   Destination code = 0x%02x", _cart[ 0x14a ] );
      _log->Write( Log::info, buffer );

      sprintf( buffer, "   Header checksum = 0x%02x", _cart[ 0x14d ] );
      _log->Write( Log::info, buffer );

      sprintf(buffer, "   Global checksum = 0x%02hhx%02hhx", _cart[ 0x14e ], _cart[ 0x14f ] );
      _log->Write( Log::info, buffer );


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
      os << setHex( 4 ) << i + startAddress << ": ";
    }

    if( i < additional ) {
      os << "  ";
    }
    else {
      os << setHex( 2 ) << ( read8( i + startAddress ) & 0xff );
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
