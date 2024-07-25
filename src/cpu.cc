
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "../include/cpu.hh"

#include "../include/bus.hh"
#include "../include/common.hh"

CPU::CPU() {
  auto keys = conf->GetKeys();

  auto hasStartDebug = std::find( keys.begin(), keys.end(), "StartInDebug" );
  if( hasStartDebug != keys.end() ) {
    preExec.push_back( &CPU::Step );
  }

  auto hasTrace = std::find( keys.begin(), keys.end(), "TraceLog");
  if( hasTrace != keys.end() ) {
    trace.open( conf->GetValue( *hasTrace ) );
    preExec.push_back( &CPU::Trace );
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
  auto signedData = static_cast< std::int8_t >( parm1 );
  bool negativeData = signedData < 0;

  if( negativeData ) {
    signedData = -signedData;
  }

  sprintf( formattedData8, "%02x", parm1 );
  sprintf( formattedSignedData8, "%s%x", negativeData ? "-" : "", signedData );
  sprintf(formattedData16, "%04x", data16);

  std::string outline{ instr.desc };

  outline = std::regex_replace( outline, std::regex( "d8" ), formattedData8 );
  outline = std::regex_replace( outline, std::regex( "d16" ), formattedData16 );
  outline = std::regex_replace( outline, std::regex( "a8" ), formattedData8 );
  outline = std::regex_replace( outline, std::regex( "a16" ), formattedData16 );
  outline = std::regex_replace( outline, std::regex( "r8" ), formattedSignedData8);

  char buffer[ 1024 ] = { 0 };

  auto flags = regs.F;

  // display the regisgers
  int offset = sprintf( buffer, "AF:%04x BC:%04x DE:%04x HL:%04x PC:%04x SP:%04x %s%s%s%s  %lu ticks\n",
           regs.AF, regs.BC, regs.DE, regs.HL, regs.PC, regs.SP,
           ( ( flags & Zmask ) > 0 ? "Z" : "z" ),
           ( ( flags & Nmask ) > 0 ? "N" : "n" ),
           ( ( flags & Hmask ) > 0 ? "H" : "h" ),
           ( ( flags & Cmask ) > 0 ? "C" : "c" ),
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

    auto cmd = dbgHandlers.find( command );
    if( cmd != dbgHandlers.end() ) {
      auto shouldReturn = ( this->*cmd->second.handle )( commandLine );
      if( shouldReturn ) {
        return;
      }
    }
    else {
      std::cout << "Unknown command " << command << std::endl;

      std::vector< std::string > keys;
      for( auto i : dbgHandlers ) {
        if( !i.second.desc.empty() ) {
          keys.push_back( i.second.desc );
        }
      }
      std::sort( keys.begin(), keys.end() );

      std::cout << "Available commands are: ";

      for( auto k : keys ) {
        std::cout << k << "; ";
      }

      std::cout << std::endl;
    }
  }
}

bool
CPU::dbgStep( std::stringstream& is ) {
  auto i = std::find( preExec.begin(), preExec.end(), &CPU::Step );

  if( i == preExec.end() ) {
    preExec.push_back( &CPU::Step );
  }

  return true;
}

bool
CPU::dbgDump( std::stringstream& is ) {
    u16 addr;
    is >> std::hex >> addr;
    auto mem = bus->hexDump( addr, 3 * 16 );
    std::cout << mem;

    return false;
}

bool
CPU::dbgBreak( std::stringstream& is ) {
  u16 addr;
  is >> std::hex >> addr;
  breakpoints[ addr ] =  bus->read( addr );
  bus->dbgWrite( addr, debugOpcode );
  std::cout << "breakpoint set at address " << setHex( 4 ) << addr <<
    std::endl;

  return false;
}

bool
CPU::dbgContinue( std::stringstream& is ) {
    if( breakpoints.size() > 0 ) {
      auto i = std::find( preExec.begin(), preExec.end(), &CPU::Step );
      preExec.erase( i );
      return true;
    }
    else {
      std::cout << "Need at least one breakpoint to continue" << std::endl;
      return false;
    }
}

bool
CPU::dbgPoke( std::stringstream& is ) {
  u16 addr;
  is >> std::hex >> addr;
  if( !is.eof() ) {
    int dataInput;
    is >> std::hex >> dataInput;

    u8 data = dataInput & 0xff;

    std::cout << "The old data at address " << setHex( 4 ) << addr <<
      " is " << setHex( 2 ) << ( bus->read( addr ) & 0xff ) << std::endl;

    bus->write( addr, data );
  }
  else {
    std::cout << "Expected 2 arguments to poke" << std::endl;
  }

  return false;
}

bool
CPU::dbgSetPC( std::stringstream& is ) {
  u16 addr;
  is >> std::hex >> addr;
  regs.PC = addr;

  ( this->*decodeHandle )();

  std::cout << debugSummary( ins_decode, params[ 0 ], params[ 1 ] ) << std::endl;

  return false;
}

void
CPU::decode() {
  addrCurrentInstr = regs.PC;
  u16 ins = bus->read( regs.PC++ );

  ins_decode = instrs[ ins ];

  if( ins_decode.bytes > 1 ) {
    for( auto i = 0; i < ins_decode.bytes - 1; i++ ) {
      params[ i ] = bus->read( regs.PC++ );
    }
  }
}

void CPU::prefixDecode() {
  addrCurrentInstr = regs.PC;
  u16 ins = bus->read( regs.PC++ );

  if( ins != debugOpcode ) {
    ins |= 0b1'0000'0000;
  }

  ins_decode = instrs[ins];

  if (ins_decode.bytes > 1) {
    for (auto i = 0; i < ins_decode.bytes - 1; i++) {
      params[i] = bus->read(regs.PC++);
    }
  }

  decodeHandle = &CPU::decode;
}

void
CPU::Trace() {
  trace << debugSummary(ins_decode, params[0], params[1]) << std::endl;
}

void
CPU::Step() {
  debug(ins_decode, params[0], params[1]);
}

void CPU::_clock() {
  ticks++;

  if( ticks >= waitUntilTicks ) {
    processInterrupts();

    ( this->*decodeHandle )();

    for( auto f : preExec ) {
      ( this->*f )();
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
    case 0x2: {
      // Load register A into memory pointed to by r16
      auto regIndex = instr.binary >> 4 & 0b111;
      auto reg = pr16_1[ regIndex ];
      if( regIndex <= 1 ) {
        bus->write( regs.*reg, regs.A );
      }
      else if( regIndex == 2 ) {
        // LD (HL+), A
        bus->write( regs.HL++, regs.A );
      }
      else if( regIndex == 3 ) {
        bus->write( regs.HL--, regs.A );
      }
      }
      break;  // block zero opcode 2
    case 0x6: {
      // Load r8 encoded in instruction with immediate d8
      auto dest = pr8[ ( instr.binary >> 3 ) & 0x7 ];
      if( dest != &Registers::F ) {
        regs.*dest = parm1;
      }
      else {
        // This is LD (HL),d8
        bus->write( regs.HL, parm1 );
      }
      }
      break;
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
      // The LD instruction with sub op code 0xe don't write to (HL)
      // so no need to check for dest of &Registers::F
      regs.*dest = parm1;
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
    auto srcPtr = pr8[ instr.binary & 0b111 ];
    auto dstPtr = pr8[ ( instr.binary >> 3 ) & 0b111 ];

    if( dstPtr != &Registers::F ) {
      regs.*dstPtr = regs.*srcPtr;
    }
    else {
      // This is a load of register to memory at (HL)
      bus->write( regs.HL, regs.*srcPtr );
    }
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
  push( regs.*( pr16_2[ registerIndex ] ) );

  return instr.cycles1;
}

u8
CPU::POP( const InstDetails& instr, u8, u8 ) {
  u8 destinationIndex = ( instr.binary >> 4 ) & 0x3;
  regs.*( pr16_2[ destinationIndex ] ) = pop();

  return instr.cycles1;
}

void
CPU::inc( int data, int amt, u8& result ) {
  // NOTE: does not change the carry flag. (?)
  auto dataH = data & 0xf;
  int intResult = data + amt;
  result = intResult & 0xff;

  bool isZ = result == 0;
  bool isH = dataH + ( amt & 0xf ) > 0xf;

  if( isZ ) {
    regs.F |= Zmask;
  }
  else {
    regs.F &= ~Zmask;
  }

  if( isH ) {
    regs.F |= Hmask;
  }
  else {
    regs.F &= ~Hmask;
  }
}

u8
CPU::INC( const InstDetails& instr, u8, u8 ) {
  // is it a 16-bit register or 8-bit register to increment?
  bool is16bit = ( instr.binary & 0x3 ) == 3;

  if( is16bit ) {
    u8 registerIndex = ( instr.binary >> 4 ) & 0b11;
    regs.*(pr16_1[registerIndex]) = (regs.*(pr16_1[registerIndex]) + 1) & 0xffff;
  }
  else {
    auto reg = pr8[ ( instr.binary >> 3 ) & 0b111 ];
    if( reg != &Registers::F ) {
      u8 result;
      inc( regs.*reg, 1, result );
      regs.*reg = result;
    }
    else {
      u8 result;
      inc( bus->read( regs.HL ), 1, result );
      bus->write( regs.HL, result );
    }
    regs.F &= ~Nmask;
  }

  return instr.cycles1;
}

u8
CPU::OR(const InstDetails& instr, u8 parm1, u8) {
  if( ( instr.binary & 0b1011'0000 ) == 0b1011'0000 ) {
    // OR the A register with another register and store the result in A
    u8 registerIndex = instr.binary & 0b0111;
    if( pr8[ registerIndex ] != &Registers::F ) {
      regs.A |= regs.*pr8[ registerIndex ];
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
    regs.F |= 0b1000'0000;
  }

  return instr.cycles1;
}

u8
CPU::DBG( const InstDetails& instr, u8, u8 ) {
  // Set things up for the call to the debug display
  u16 op_code = breakpoints[ addrCurrentInstr ];

  if( prefixForAddress == addrCurrentInstr ) {
    op_code |= 0b1'0000'0000;
  }

  ins_decode = instrs[ op_code ];

  // Read arguments, if any
  if( ins_decode.bytes > 1 ) {
    for (auto i = 0; i < ins_decode.bytes - 1; i++) {
      params[ i ] = bus->read( regs.PC++ );
    }
  }

  if( trace.is_open() ) {
    trace << debugSummary( ins_decode, params[ 0 ], params[ 1 ] ) << std::endl;
  }

  std::cout << "Hit breakpoint at " << setHex( 4 ) << addrCurrentInstr << std::endl;
  debug( ins_decode, params[ 0 ], params[ 1 ] );

  return( this->*ins_decode.impl )( ins_decode, params[ 0 ], params[ 1 ] );
}

void
CPU::add( int parm1, int parm2, u8& result ) {
  int intResult = parm1 + parm2;
  result = intResult & 0xff;

  bool isZ = result == 0;
  bool isH = ( parm1 & 0x0f ) + ( parm2 & 0x0f ) > 0x0f;
  bool isC = static_cast< unsigned >( intResult ) > 0xff;

  regs.F = ( isZ * Zmask ) | ( isH * Hmask ) | ( isC * Cmask );
}

u8
CPU::CP( const InstDetails& instr, u8 parm1, u8 )    {
  u8 result;

  // Which CP op code are we looking at
  auto block = ( instr.binary >> 6 ) & 0x3;

  switch( block ) {
  case 2: {
    // Compare register A with other register
    auto otherReg = pr8[ instr.binary & 0x7 ];
    if( otherReg != &Registers::F ) {
      add( regs.A, -( regs.*otherReg ), result );
    }
    else {
      // Compare register A with data in memory location (HL)
      add( regs.A, -( bus->read( regs.HL ) & 0xff ), result );
    }
  }
    break;
  case 3:
    // Compare register A with data
    add( regs.A, -parm1, result );
    break;
  default: {
    char buffer[ 1024 ] = { 0 };
    sprintf( buffer, "In CPU::CP op code with invalid op code: 0x%02x, PC = 0x%04x",
             instr.binary, addrCurrentInstr );
    throw std::runtime_error( buffer );
  }
    break;
  }

  regs.F |= Nmask;

  return instr.cycles1;
}

u8
CPU::AND(const InstDetails& instr, u8 param1, u8 ) {

  if( ( instr.binary & 0xC0 ) != 0xC0 ) {
    // With register
    auto otherReg = pr8[ instr.binary & 0x7 ];
    if( otherReg != &Registers::F ) {
      regs.A &= regs.*otherReg;
    }
    else {
      // With data at address in (HL)
      auto data = bus->read( regs.HL ) & 0xff;
      regs.A &= data;
    }
  }
  else {
    // With immediate data
    regs.A &= param1;
  }

  if( regs.A == 0 ) {
    regs.F |= Zmask;
  }

  regs.F |= Hmask;

  return instr.cycles1;
}

u8
CPU::DEC( const InstDetails& instr, u8, u8 ) {

  switch( instr.binary & 0b111 ) {
  case 3: {
    // DEC r16 NOTE: does not update flags
    regs.*pr16_1[ ( instr.binary >> 4 ) & 0x3 ] -= 1;
    }
    break;

  case 5: {
    // DEC r8 NOTE: DOES update flags
    auto reg = pr8[ ( instr.binary >> 3 ) & 0x7 ];

    if( reg != &Registers::F ) {
      u8 result;
      inc( regs.*reg, -1, result );
      regs.*reg = result;
    }
    else {
      // This is really DEC (HL)
      u8 result;
      inc( bus->read( regs.HL ), -1, result );
      bus->write( regs.HL, result );
    }

    regs.F |= Nmask;
  }
    break;

  default: {
    char buffer[ 1024 ] = { 0 };
    sprintf( buffer, "In DEC with invalid opcode 0x%02x from address 0x%04x",
             instr.binary, addrCurrentInstr );
    _log->Write( Log::warn, buffer );
  }
    break;
  }

  return instr.cycles1;
}

u8
CPU::XOR( const InstDetails& instr, u8 parm1, u8 ) {
  bool isD8 = (instr.binary & 0b1100'0000) == 0b1100'0000;
  regs.F = 0;

  if( isD8 ) {
    u8 result = regs.A ^ parm1;
    regs.A = result;
    if( result == 0 ) {
      regs.F |= Zmask;
    }
  }
  else {
    auto reg = pr8[ instr.binary & 0x7 ];
    if( reg != &Registers::F ) {
      regs.A ^= regs.*reg;
    }
    else {
      // This is really regs.A ^ (HL)
      u8 data = bus->read( regs.HL ) & 0xff;
      regs.A ^= data;
    }

    if (regs.A == 0) {
      regs.F |= Zmask;
    }
  }

  return instr.cycles1;
}

u8
CPU::ADD( const InstDetails& instr, u8 parm1, u8 ) {
  bool isD8 = ( instr.binary & 0xc0 ) == 0xc0;
  u8 result;

  regs.F = 0;

  if( isD8 ) {
    add( regs.A, parm1, result );
  }
  else {
    auto otherReg = pr8[ instr.binary & 0b111 ];

    if( otherReg != &Registers::F ) {
      add( regs.A, regs.*otherReg, result );
    }
    else {
      // This is ADD A,(HL)
      u8 data = bus->read( regs.HL ) & 0xff;
      add( regs.A, data, result );
    }
  }

  regs.A = result;

  return instr.cycles1;
}

u8
CPU::SUB( const InstDetails& instr, u8 parm1, u8 ) {
  bool isD8 = ( instr.binary & 0xc0 ) == 0xc0;
  u8 result;

  regs.F = 0;

  if( isD8 ) {
    add( regs.A, -( parm1 & 0xff ), result );
  }
  else {
    auto otherReg = pr8[ instr.binary & 0b111 ];

    if( otherReg != &Registers::F ) {
      add( regs.A, -( regs.*otherReg ), result );
    }
    else {
      // This is ADD A,(HL)
      add(regs.A, -( bus->read( regs.HL ) & 0xff ), result);
    }
  }

  regs.A = result;
  regs.F |= Nmask;

  return instr.cycles1;
}

u8
CPU::PREFIX(const InstDetails& instr, u8, u8) {

  prefixForAddress = regs.PC;
  decodeHandle = &CPU::prefixDecode;

  return instr.cycles1;
}

u8
CPU::SRL( const InstDetails& instr, u8, u8 ) {

  auto reg = pr8[ instr.binary & 0b111 ];
  regs.F = 0;

  if( reg != &Registers::F ) {
    if( regs.*reg & 0b1 ) {
      regs.F |= Cmask;
    }

    regs.*reg >>= 1;

    if( regs.*reg == 0 ) {
      regs.F |= Zmask;
    }
  }
  else {
    // This is SRL (HL)
    u8 data = bus->read( regs.HL );
    if( data & 0b1 ) {
      regs.F |= Cmask;
    }

    data >>= 1;

    if( data == 0 ) {
      regs.F |= Zmask;
    }

    bus->write( regs.HL, data );
  }

  return instr.cycles1;
}

u8 CPU::rotateRight( u8 data ) {
  bool oldBit0 = data & 0b1;

  regs.F = 0;

  data >>= 1;

  if( oldBit0 ) {
    regs.F |= Cmask;
    data |= 0b1000'0000;
  }

  if( data == 0 ) {
    regs.F |= Zmask;
  }

  return data;
}

u8
CPU::rotateRightC( u8 data ) {
  bool oldBit0 = ( data & 0b1 ) > 0;
  bool oldFlagC = ( regs.F & Cmask ) > 0;

  regs.F = 0;

  data >>= 1;

  if( oldBit0 ) {
    regs.F |= Cmask;
  }

  if( oldFlagC ) {
    data |= 0b1000'0000;
  }

  if( data == 0 ) {
    regs.F |= Zmask;
  }

  return data;
}

u8
CPU::RR( const InstDetails& instr, u8, u8 ) {

  auto reg = pr8[ instr.binary & 0b111 ];

  if( reg != &Registers::F ) {
    regs.*reg = rotateRightC( regs.*reg );
  }
  else {
    throw std::runtime_error( "RR (HL) not implemented" );
  }

  return instr.cycles1;
}

u8
CPU::RRA( const InstDetails &instr, u8, u8 ) {

  regs.A = rotateRightC( regs.A );

  return instr.cycles1;
}

u8
CPU::ADC( const InstDetails &instr, u8 parm1, u8 ) {
  u8 block = instr.binary >> 6;
  u8 result;

  switch( block ) {
  case 2: {
    // Add register and carry to register A
    auto srcReg = pr8[ instr.binary & 0x7 ];
    result = regs.*srcReg;
    if( regs.F & Cmask ) {
      add( regs.*srcReg, 1, result );
    }
    add( result, regs.A, result );
  }

    break;
  case 3: {
    // Add immediate with carry to register A
    parm1 &= 0xff;
    if( regs.F & Cmask ) {
      add( parm1, 1, result );
    }
    add( parm1, regs.A, result );
  }

    break;
  default:
    throw std::runtime_error( "In ADC instruction with invalid opcode" );
    break;
  }

  regs.A = result;

  return instr.cycles1;
}
