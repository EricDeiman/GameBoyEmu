#include <vector>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <ios>

#include "../include/cpu.hh"
#include "../include/common.hh"

void
CPU::debug( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  char buffer[ 1024 ] = { 0 };

  int offset = sprintf( buffer, "0x%04x:  %s ", addrCurrentInstr, instr.desc.c_str() );

  if( instr.bytes > 1 ) {
    offset += sprintf( buffer + offset, " 0x%02x", parm1 );
  }

  if( instr.bytes == 3 ) {
    sprintf( buffer + offset, " 0x%02x", parm2);
  }

  _log->Write( Log::debug, buffer );
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

    debug( ins_decode, params[ 0 ], params[ 1 ] );
    auto cycleCnt = ( this->*ins_decode.impl )( ins_decode, params[ 0 ], params[ 1 ] );

    waitUntilTicks = ticks + cycleCnt - 1;
  }

}

u8
CPU::NOP( const InstDetails &instr, u8 parm1, u8 parm2 ) {
  return instr.cycles1;
}

u8
CPU::JP( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  // Remember, the SM83, along with the 8080 and Z80 are little endian
  regs.PC = ( (u16)parm2 << 8 ) | ( (u16)parm1 & 0xff );

  // std::stringstream ss;
  // ss << std::hex;
  // ss << "In JP instruction at 0x" << std::setfill( '0' ) << std::setw( 4 ) <<
  //   addrCurrentInstr <<
  //   ", parm1 = 0x" << std::setfill( '0' ) << std::setw( 2 ) << ( parm1 & 0xff ) <<
  //   ", parm2 = 0x" << std::setfill( '0' ) << std::setw( 2 ) << ( parm2 & 0xff ) <<
  //   ", jumping to 0x" << std::setfill( '0' ) << std::setw( 4 ) << regs.PC;

  // _log->Write(Log::info, ss.str() );

  return instr.cycles1;
}

u8
CPU::DI( const InstDetails& instr, u8 parm1, u8 parm2 ) {
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

      char buffer[ 1024 ] = { 0 };
      sprintf( buffer, "\tIn LD r16, d16, register offset = %d, data = 0x%04x",
               reg, data );
      _log->Write( Log::debug, buffer );

      this->regs.*dest = data;
    }    
      break;
    case 0x2:
      throw std::runtime_error( "Block zero LD instruction 0x2 not implemented" );
      break;
    case 0x8:
      throw std::runtime_error( "Block zero LD instruction 0x8 not implemented" );
      break;
    case 0xa:
      throw std::runtime_error( "Block zero LD instruction 0xa not implemented" );
      break;
    default:
      throw std::runtime_error( "Block zero LD instruction not implemented" );
      break;
    }
  }
    break;
  default:
    throw std::runtime_error( "LD instruction not implemented" );
    break;
  }

  return instr.cycles1;
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
