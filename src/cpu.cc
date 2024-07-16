#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include "../include/cpu.hh"
#include "../include/common.hh"

CPU::CPU( Bus* bus ) : bus{ bus } {
  auto keys = conf->GetKeys();

  auto hasStartDebug = std::find( keys.begin(), keys.end(), "StartInDebug" );

  if( hasStartDebug != keys.end() ) {
    singleStepMode = conf->GetValue( "StartInDebug" ) == "true";
  }

  regs.PC = 0x100;
}

std::string
CPU::debugSummary( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  u16 data16 = ( parm2 << 8 ) | parm1;
  char formattedData8[ 32 ];
  char formattedData16[ 32 ];

  sprintf( formattedData8, "%02x", parm1 );
  sprintf( formattedData16, "%04x", data16 );

  std::string outline{ instr.desc };

  outline = std::regex_replace( outline, std::regex( "d8" ), formattedData8 );
  outline = std::regex_replace( outline, std::regex( "d16" ), formattedData16 );
  outline = std::regex_replace( outline, std::regex( "a8" ), formattedData8 );
  outline = std::regex_replace( outline, std::regex( "a16" ), formattedData16 );

  char buffer[ 1024 ] = { 0 };

  auto Zmask = 0b1000'0000;
  auto Nmask = 0b0100'0000;
  auto Hmask = 0b0010'0000;
  auto Cmask = 0b0001'0000;

  auto flags = regs.F;

  // display the regisgers
  int offset = sprintf( buffer, "AF:%04x BC:%04x DE:%04x HL:%04x PC:%04x SP:%04x %s%s%s%s\n",
           regs.AF, regs.BC, regs.DE, regs.HL, regs.PC, regs.SP,
           ( (flags & Zmask) > 0 ? "Z" : "z" ),
           ( (flags & Nmask) > 0 ? "N" : "n" ),
           ( (flags & Hmask) > 0 ? "H" : "h" ),
           ( (flags & Cmask) > 0 ? "C" : "c" ) );

  offset += sprintf( buffer + offset, "0x%04x:  %02x", addrCurrentInstr, instr.binary );

  if( instr.bytes == 1 ) {
    offset += sprintf( buffer + offset, "\t\t" );
  }
  else if ( instr.bytes == 2 ) {
    offset += sprintf( buffer + offset, " %02x\t\t", parm1 );
  }
  else if( instr.bytes == 3 ) {
    offset += sprintf( buffer + offset, " %02x %02x\t", parm1, parm2 );
  }

  offset += sprintf( buffer + offset, "  %s", outline.c_str() );


  //_log->Write( Log::debug, buffer );
  return buffer;
}

// This method halts everything until it return.
void
CPU::debug( const InstDetails& instr, u8 parm1, u8 parm2 ) {

  std::cout << debugSummary( instr, parm1, parm2 ) << std::endl;

  for(;;) {
    std::cout << "> ";
    std::string input;
    std::getline( std::cin, input );

    std::stringstream commandLine{ input };
    std::string command;

    commandLine >> command;

    if( command == "step" || command == "s" ) {
      return;
    }
    else if( command == "dump" || command == "d" ) {
      u16 addr;
      commandLine >> std::hex >> addr;
      auto mem = bus->hexDump( addr, 3 * 16 );
      std::cout << mem;
    }
    else {
      std::cout << "Available commands: (s)tep, (h)elp, (d)ump <address>" << std::endl;
    }
  }


}

void
CPU::_clock() {
  ticks++;

  if( ticks >= waitUntilTicks ) {
    u8 params[2]{ 0 };

    // Read the instruction at PC
    addrCurrentInstr = regs.PC;

    u8 ins = bus->read( regs.PC++ );

    auto ins_decode = instrs[ ins ];

    if( ins_decode.bytes > 1 ) {
      for( auto i = 0; i < ins_decode.bytes - 1; i++ ) {
        params[ i ] = bus->read( regs.PC++ );
      }
    }

    if( singleStepMode ) {
      debug( ins_decode, params[ 0 ], params[ 1 ] );      
    }

    auto cycleCnt = ( this->*ins_decode.impl )( ins_decode, params[ 0 ], params[ 1 ] );

    waitUntilTicks = ticks + cycleCnt - 1;
  }

}

u8
CPU::NOP( const InstDetails &instr, u8, u8 ) {
  return instr.cycles1;
}

u8
CPU::JP( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  // Remember, the SM83, along with the 8080 and Z80 are little endian
  regs.PC = ( (u16)parm2 << 8 ) | ( (u16)parm1 & 0xff );

  return instr.cycles1;
}

u8
CPU::DI( const InstDetails& instr, u8, u8 ) {
  interruptsEnabled = false;

  return instr.cycles1;
}

u8
CPU::LD( const InstDetails& instr, u8 parm1, u8 parm2 ) {

  // There are many load instructions, which one are we dealing with?
  int block = ( instr.binary >> 6 ) & 0x3;
  switch( block ) {
  case 0: {
    int b0opcode = instr.binary & 0xf;
    switch( b0opcode ) {
    case 0x1: {
      // Load d16 into a 16-bit register.
      int reg = ( instr.binary >> 4 & 0x3 );
      auto dest = p16[ reg ];
      u16 data = ( parm2 << 8 ) | parm1;

      this->regs.*dest = data;
    } // block zero opcode zero
      break;
    case 0x2:
      throw std::runtime_error( "Block zero LD instruction 0x2 not implemented" );
      break;  // block zero opcode 2
    case 0x8:
      throw std::runtime_error( "Block zero LD instruction 0x8 not implemented" );
      break;  // block zero opcode 8
    case 0xa:
      throw std::runtime_error( "Block zero LD instruction 0xa not implemented" );
      break;  // block zero opcode a
    case 0xe: {
      // Load  d8 into 8-bit register
      int reg = ( instr.binary >> 3 ) & 0x7;
      auto dest = pr8[ reg ];

      this->regs.*dest = parm1;
    }
      break;  // block zero opcode e
    default:
      throw std::runtime_error( "Block zero LD instruction not implemented" );
      break;
    }
  }  // block zero
    break;
  case 3: {
    switch( instr.binary ) {
    case 0xea: {
      // load the contents of the A register at memory location in the parms
      u16 a16 = ( parm2 << 8 ) | parm1;

      bus->write( a16, regs.A );
    }  // block 3 opcode ea
      break;
    }
  } // block 3
    break;
  default:
    throw std::runtime_error( "LD instruction not implemented" );
    break;
  }

  return instr.cycles1;
}

u8
CPU::LDH( const InstDetails& instr, u8 parm1, u8 ) {
  u16 addr = 0xff00 | ( parm1 & 0xff );

  bus->write( addr, regs.A );

  return instr.cycles1;
}


// CALL NZ, a16 (condition code is 0x0)
// CALL  Z, a16 (condition cdoe is 0x1)
// CALL NC, a16 (condition code is 0x2)
// CALL  C, a16 (condition code is 0x3)
u8
CPU::CALL( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  u16 callAddress = ( parm2 << 8 ) | parm1;
  u8 returnAddressLo = regs.PC & 0xff;
  u8 returnAddressHi = ( regs.PC >> 8 ) & 0xff;

  u8 opcode = instr.binary & 0x7;
  u8 conditionCode = ( instr.binary >> 3 ) & 3;

  switch( opcode ) {
  case 0x4: {
    // conditional call
    u8 Zmask = 0x80;
    u8 Cmask = 0x10;

    bool isZ = (regs.F & Zmask) > 0;
    bool isC = (regs.F & Cmask) > 0;

    bool doCall = false;

    switch( conditionCode ){
    case 0:
      if( !isZ ) {
        doCall = true;
      }
      break;
    case 1:
      if( isZ ) {
        doCall = true;
      }
      break;
    case 2:
      if( !isC ) {
        doCall = true;
      }
      break;
    case 3:
      if( isC ) {
        doCall = true;
      }
      break;
    }

    if( doCall ) {
      bus->write( --regs.SP, returnAddressLo );
      bus->write( --regs.SP, returnAddressHi );
      
      regs.PC = callAddress;

      return instr.cycles1;
    }
    else {
      return instr.cycles2;
    }
  }
    break;
  case 0x5:
    // unconditional call  
    bus->write( --regs.SP, returnAddressLo );
    bus->write( --regs.SP, returnAddressHi );

    regs.PC = callAddress;

    return instr.cycles1;

    break;
  }

  throw std::runtime_error( "At end of CPU::CALL with invalid opcode" );
}

u8
CPU::ADC( const InstDetails &instr, u8 parm1, u8 parm2 ) {
  u8 adc_a_r8 = 0b1000'1000;
  u8 Areg_idx = 0b111;

  u8 Registers::*dest_reg = pr8[ Areg_idx ];

  u8 src_data;
  u8 dest_data = regs.*dest_reg;

  if( ( static_cast< u8 >( instr.binary ) & adc_a_r8 ) == adc_a_r8 ) {
    // ADC A, r8
    u8 Registers::*src_reg = pr8[ static_cast< u8 >( instr.binary ) & 0b111 ];
    src_data = regs.*src_reg;
  }
  else {
    // ADC A, d8
    // src_data = parm1;
  }

  return instr.cycles1;
}
