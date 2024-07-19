#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "../include/cpu.hh"

#include "../include/bus.hh"
#include "../include/common.hh"

CPU::CPU() {
  auto keys = conf->GetKeys();

  auto hasStartDebug = std::find( keys.begin(), keys.end(), "StartInDebug" );

  if( hasStartDebug != keys.end() ) {
    singleStepMode = conf->GetValue( "StartInDebug" ) == "true";
  }

  regs.PC = 0x100;

}

void
CPU::initialize( Bus *bus ) {
  this->bus = bus;
}

std::string
CPU::debugSummary( const InstDetails &instr, u8 parm1, u8 parm2 ) {
  u16 data16 = ( parm2 << 8 ) | parm1;
  char formattedSignedData8[ 32 ];
  char formattedData8[ 32 ];
  char formattedData16[ 32 ];

  sprintf( formattedData8, "%02x", parm1 );
  sprintf( formattedSignedData8, "%d", parm1 );
  sprintf(formattedData16, "%04x", data16);

  std::string outline{ instr.desc };

  outline = std::regex_replace( outline, std::regex( "d8" ), formattedData8 );
  outline = std::regex_replace( outline, std::regex( "d16" ), formattedData16 );
  outline = std::regex_replace( outline, std::regex( "a8" ), formattedData8 );
  outline = std::regex_replace( outline, std::regex( "a16" ), formattedData16 );
  outline = std::regex_replace( outline, std::regex( "r8" ), formattedSignedData8);

  char buffer[ 1024 ] = { 0 };

  auto Zmask = 0b1000'0000;
  auto Nmask = 0b0100'0000;
  auto Hmask = 0b0010'0000;
  auto Cmask = 0b0001'0000;

  auto flags = regs.F;

  // display the regisgers
  int offset = sprintf( buffer, "AF:%04x BC:%04x DE:%04x HL:%04x PC:%04x SP:%04x %s%s%s%s  %lu ticks\n",
           regs.AF, regs.BC, regs.DE, regs.HL, regs.PC, regs.SP,
           ( (flags & Zmask) > 0 ? "Z" : "z" ),
           ( (flags & Nmask) > 0 ? "N" : "n" ),
           ( (flags & Hmask) > 0 ? "H" : "h" ),
           ( (flags & Cmask) > 0 ? "C" : "c" ),
           ticks
           );

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
      singleStepMode = true;
      return;
    }
    else if( command == "dump" || command == "d" ) {
      u16 addr;
      commandLine >> std::hex >> addr;
      auto mem = bus->hexDump( addr, 3 * 16 );
      std::cout << mem;
    }
    else if( command == "break" || command == "b" ) {
      u16 addr;
      commandLine >> std::hex >> addr;
      breakpoints[ addr ] =  bus->read( addr );
      bus->write( addr, debugOpcode );
      std::cout << "breakpoint set at address " << setHex( 4 ) << addr << std::endl;
    }
    else if( command == "continue" || command == "c" ) {
      if( breakpoints.size() > 0 ) {
        singleStepMode = false;
        return;
      }
      else {
        std::cout << "Need at least one breakpoint to continue" << std::endl;
      }
    }
    else if( command == "poke" || command == "p" ) {
      u16 addr;
      commandLine >> std::hex >> addr;
      if( !commandLine.eof() ) {
        int dataInput;
        commandLine >> std::hex >> dataInput;

        u8 data = dataInput & 0xff;

        std::cout << "The old data at address " << setHex( 4 ) << addr <<
          " is " << setHex( 2 ) << ( bus->read( addr ) & 0xff ) << std::endl;

        bus->write( addr, data );
      }
      else {
        std::cout << "Expected 2 arguments to poke" << std::endl;
      }
    }
    else {
      std::cout << "Available commands: (s)tep, (h)elp, (d)ump <address>, "
                << "(b)reak <address>, (c)ontinue, (p)oke <address> <value>" << std::endl;
    }
  }
}

void
CPU::_clock() {
  ticks++;

  if( ticks >= waitUntilTicks ) {
    processInterrupts();

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

    waitUntilTicks = ticks + cycleCnt;
  }

}

u8
CPU::NOP( const InstDetails &instr, u8, u8 ) {
  return instr.cycles1;
}

bool
CPU::checkCondCode( u8 condCode ) {
  u8 Zmask = 0x80;
  u8 Cmask = 0x10;

  bool isZ = (regs.F & Zmask) > 0;
  bool isC = (regs.F & Cmask) > 0;

  switch (condCode) {
  case 0:
    return !isZ;
  case 1:
    return isZ;
  case 2:
    return !isC;
  case 3:
    return isC;
  }

  return false;
}

u8
CPU::JP( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  // Remember, the SM83, along with the 8080 and Z80 are little endian

  switch( instr.binary & 0b11 ) {
  case 1:  // JP (HL)
    regs.PC = bus->read( regs.HL );
    break;
  case 2: {
    bool doJump = checkCondCode( ( instr.binary >> 3 ) & 0b11 );

    if ( doJump ) {
      push(regs.PC);
      regs.PC = ( ( u16 )parm2 << 8 ) | parm1;
      return instr.cycles1;
    }
    else {
      return instr.cycles2;
    }
  }
    break;
  case 3:  // Unconditional jump to address
    regs.PC = ((u16)parm2 << 8) | ((u16)parm1 & 0xff);
    break;
  }

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
      auto dest = pr16_1[ reg ];
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
    case 0xa: {
      // Load register A from memory pointed to by 16-bit register
      u8 sourceReg = (instr.binary >> 4) & 0b11;
      switch( sourceReg ) {
      case 0x0:  // from register (BC)
        regs.A = bus->read( regs.BC );
        break;
      case 0x1:  // from register (DE)
        regs.A = bus->read( regs.DE );
        break;
      case 0x2:  // from register (HL+), increment HL after read
        regs.A = bus->read( regs.HL++ );
        break;
      case 0x3:  // from register (HL-), decrement HL after read
        regs.A = bus->read( regs.HL-- );
        break;
      }
    }
      break;  // block zero opcode a
    case 0xe: {
      // Load  d8 into 8-bit register
      int reg = ( instr.binary >> 3 ) & 0x7;
      auto dest = pr8[ reg ];

      this->regs.*dest = parm1;
    }
      break;  // block zero opcode e
    default: {
      char buffer[ 1024 ] = { 0 };
      sprintf( buffer, "Block zero LD instruction 0x%02x not implemented", b0opcode );
      throw std::runtime_error( buffer );
      }
      break;
    }
  }  // block zero
    break;
  case 1: {
    // load register to register
    u8 srcIndex = instr.binary & 0b111;
    u8 dstIndex = ( instr.binary >> 3 ) & 0b111;

    auto srcPtr = pr8[ srcIndex ];
    auto dstPtr = pr8[ dstIndex ];

    this->regs.*dstPtr = this->regs.*srcPtr;
  } // block one
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
  default: {
    char buffer[ 1024 ] = { 0 };
    sprintf( buffer, "LD instruction 0x%02x not implemented", instr.binary );
    throw std::runtime_error( buffer );
    }
    break;
  }

  return instr.cycles1;
}

u8
CPU::LDH( const InstDetails& instr, u8 parm1, u8 ) {
  u16 addr = 0xff00 | ( parm1 & 0xff );

  if( ( instr.binary & 0b0001'0000 ) == 0 ) {
    bus->write(addr, regs.A);
  }
  else {
    regs.A = bus->read( addr );
  }

  return instr.cycles1;
}


// CALL NZ, a16 (condition code is 0x0)
// CALL  Z, a16 (condition cdoe is 0x1)
// CALL NC, a16 (condition code is 0x2)
// CALL  C, a16 (condition code is 0x3)
u8
CPU::CALL( const InstDetails& instr, u8 parm1, u8 parm2 ) {
  u16 callAddress = ( parm2 << 8 ) | parm1;

  u8 opcode = instr.binary & 0x7;
  u8 conditionCode = ( instr.binary >> 3 ) & 3;

  switch( opcode ) {
  case 0x4: {
    // conditional call
    bool doCall = checkCondCode( conditionCode );

    if( doCall ) {
      push( regs.PC );
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
    push( regs.PC );
    regs.PC = callAddress;
    return instr.cycles1;
    break;
  }

  char buffer[ 1024 ] = { 0 };
  sprintf( buffer, "At end of CPU::CALL with invalid opcode 0x%02x", instr.binary );
  throw std::runtime_error( buffer );
}

void
CPU::triggerInterrupt( CPU::Interrupt interrupt ) {
  bus->write( Bus::IOAddress::IF, bus->read( Bus::IOAddress::IF ) | interrupt );
}

void
CPU::processInterrupts() {
  if( interruptsEnabled ) {
    throw std::runtime_error( "CPU::processInterrupts not implemented yet" );

    // interruptEnable = bus->read( 0xffff );
    // The priority of interrupts is 1, 2, 4, 8, 16 in that order
    // interruptRequest = bus->read( Bus::IOAddress::IF );
    // for( auto i = 0b0000'0001; i < 0b0010'0000; i << 1 ) {
    //   if( (interruptEnable & i > 1) && (interruptRequest & i > 1) ) {
    //     push regs.PC onto stack
    //     regs.PC = interrputHandler[ i ]
    //     processingingInterrupt = i
    //     break
    //   }
    // }
  }
}

void
CPU::push( u16 address ) {
  // Emulating a little-endian machine on a little-endian machine can be confusing.
  // The low and high order bytes below are really correct--they do not need to be swapped
  u8 addressHi = address & 0xff;
  u8 addressLo = ( address >> 8 ) & 0xff;

  bus->write( regs.SP--, addressLo );
  bus->write( regs.SP--, addressHi );
}

u16
CPU::pop() {
  // See note in push method about swapping (or not) the bytes when emulating little-endian
  // hardware on little-endian hardware.
  u8 addressHi = bus->read( ++regs.SP );
  u8 addressLo = bus->read( ++regs.SP );

  return ( addressLo << 8 ) | addressHi;
}

u8
CPU::JR( const InstDetails& instr, u8 parm1, u8 ) {
  // Jump relative to the current PC (program counter).  Parm1 is a signed 8-bit offset
  // that's added to PC.

  auto conditional = ( instr.binary & 0b0010'0000 ) > 0;

  auto offset = static_cast< std::int8_t >( parm1 );

  if( conditional ) {
    bool doJump = checkCondCode( ( instr.binary >> 3 ) & 0b11 );

    if( doJump ) {
      regs.PC += offset;
      return instr.cycles1;
    }
    else {
      return instr.cycles2;
    }
  }
  else {
    regs.PC += offset;
    return instr.cycles1;
  }
}

u8
CPU::RET( const InstDetails& instr, u8, u8 ) {

  if( instr.binary & 0x1 ) {
    // unconditional return
    regs.PC = pop();
    return instr.cycles1;
  }
  else {
    // conditional return
    throw std::runtime_error( "Conditional return not yet implemented" );
  }

  return 0;
}

u8
CPU::PUSH( const InstDetails& instr, u8, u8 ) {
  // source register is in bits 4 and 5
  u8 registerIndex = ( instr.binary >> 4 ) & 0x3;
  push( this->regs.*( pr16_2[ registerIndex ] ) );

  return instr.cycles1;
}

u8
CPU::POP( const InstDetails& instr, u8, u8 ) {
  u8 destinationIndex = ( instr.binary >> 4 ) & 0x3;
  this->regs.*( pr16_2[ destinationIndex ] ) = pop();

  return instr.cycles1;
}

u8
CPU::INC( const InstDetails& instr, u8, u8 ) {
  // is it a 16-bit register or 8-bit register to increment?
  bool is16bit = ( instr.binary & 0x3 ) == 3;

  if( is16bit ) {
    u8 registerIndex = ( instr.binary >> 4 ) & 0b11;
    this->regs.*(pr16_1[registerIndex]) = (this->regs.*(pr16_1[registerIndex]) + 1) & 0xffff;
  }
  else {
    u8 registerIndex = ( instr.binary >> 3 ) & 0b111;
    this->regs.*(pr8[ registerIndex]) = (this->regs.*(pr8[ registerIndex]) + 1) & 0xff;
  }

  return instr.cycles1;
}

u8
CPU::OR(const InstDetails& instr, u8 parm1, u8) {
  if( ( instr.binary & 0b1011'0000 ) == 0b1011'0000 ) {
    // OR the A register with another register and store the result in A
    u8 registerIndex = instr.binary & 0b0111;
    if( pr8[ registerIndex ] != &Registers::F ) {
      regs.A |= this->regs.*pr8[ registerIndex ];
    }
    else {
      // This is really OR (HL)
      regs.A |= bus->read( regs.HL );
    }
  }
  else {
    regs.A |= parm1;
  }

  regs.F = 0;

  if( regs.A == 0 ) {
    regs.F &= 0b1000'0000;
  }

  return instr.cycles1;
}

u8
CPU::DBG( const InstDetails& instr, u8, u8 ) {
  // Set things up for the call to the debug display
  InstDetails realInstr = instrs[ breakpoints[ addrCurrentInstr ] ];

  u8 args[ 2 ] = { 0 };

  // Read arguments, if any
  if( realInstr.bytes > 1 ) {
    for (auto i = 0; i < realInstr.bytes - 1; i++) {
      args[i] = bus->read(regs.PC++);
    }
  }

  std::cout << "Hit breakpoint at " << setHex( 4 ) << addrCurrentInstr << std::endl;
  debug( realInstr, args[ 0 ], args[ 1 ] );

  return( this->*realInstr.impl )( realInstr, args[ 0 ], args[ 1 ] );
}

// u8
// CPU::ADC( const InstDetails &instr, u8 parm1, u8 parm2 ) {
//   u8 adc_a_r8 = 0b1000'1000;
//   u8 Areg_idx = 0b111;

//   u8 Registers::*dest_reg = pr8[ Areg_idx ];

//   u8 src_data;
//   u8 dest_data = regs.*dest_reg;

//   if( ( static_cast< u8 >( instr.binary ) & adc_a_r8 ) == adc_a_r8 ) {
//     // ADC A, r8
//     u8 Registers::*src_reg = pr8[ static_cast< u8 >( instr.binary ) & 0b111 ];
//     src_data = regs.*src_reg;
//   }
//   else {
//     // ADC A, d8
//     // src_data = parm1;
//   }

//   return instr.cycles1;
// }
