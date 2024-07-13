#ifndef __cpu_hh__
#define __cpu_hh_

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

#include "common.hh"
#include "bus.hh"

class CPU {
public:

  CPU( Bus* bus ) : bus{ bus }, regs{ .PC = 0x100 } {}
  void _clock();
  u16 addrCurrentInstr = 0;
  uint64_t ticks = 1;
  uint64_t waitUntilTicks = 1;
  bool interruptsEnabled = true;

  struct Registers {
    union {
      struct {
        u8 B;
        u8 C;
      };
      u16 BC;
    };

    union {
      struct {
        u8 D;
        u8 E;
      };
      u16 DE;
    };

    union {
      struct {
        u8 H;
        u8 L;
      };
      u16 HL;
    };

    union {
      struct {
        u8 A;
        u8 F;
      };
      u16 AF;
    };

    u16 SP;
    u16 PC;
  } regs;

  // TODO: remove AddressingModes, not as helpful as I though it would be.
  enum AddressingModes {
    am_ins, // instruction encoding has all the necessary info
    am_d8,  // the argument is 1-byte data
    am_d16, // the argument is 2-byte data
    am_r8,  // the argument is an 1-byte register
    am_a16, // the argument is a 2-byte address
    am_ia8, // the argument is a 1-byte indirect address
    am_ia16 // the argument is a 2-byte indirect address
  };

  struct InstDetails {
    u16 binary;         // actual op-code bits
    std::string desc;         // description of the opcode from docs
    u8 ( CPU::*impl )( const InstDetails &, u8, u8 ); // pointer to function to interpret the op-code
    AddressingModes am; // addressing mode of opcode
    int bytes; // number of bytes used encode entire instruction and arguments
    int cycles1; // number of cycles instruction uses
    int cycles2; // alternate number of cycles instruction takes
    char Z;      // zero flag
    char N;      // subtract flag
    char H;      // half carry flag
    char C;      // carry flag
  };

  u8 ADC( const InstDetails&, u8, u8 ); // { throw std::runtime_error( "ADC not implemented" ); }
  u8 ADD( const InstDetails&, u8, u8 ) { throw std::runtime_error( "ADD not implemented" ); }
  u8 AND( const InstDetails&, u8, u8 ) { throw std::runtime_error( "AND not implemented" ); }
  u8 BIT( const InstDetails&, u8, u8 ) { throw std::runtime_error( "BIT not implemented" ); }
  u8 CALL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "CALL not implemented" ); }
  u8 CCF( const InstDetails&, u8, u8 ) { throw std::runtime_error( "CCF not implemented" ); }
  u8 CP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "CP not implemented" ); }
  u8 CPL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "CPL not implemented" ); }
  u8 DAA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "DAA not implemented" ); }
  u8 DBG( const InstDetails&, u8, u8 ) { throw std::runtime_error( "DBG not implemented" ); }
  u8 DEC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "DEC not implemented" ); }
  u8 DI( const InstDetails&, u8, u8 ); // { throw std::runtime_error( "DI not implemented" ); }
  u8 EI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "EI not implemented" ); }
  u8 HALT( const InstDetails&, u8, u8 ) { throw std::runtime_error( "HALT not implemented" ); }
  u8 ILL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "ILL not implemented" ); }
  u8 INC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "INC not implemented" ); }
  u8 JP( const InstDetails&, u8, u8 ); // { throw std::runtime_error( "JP not implemented" ); }
  u8 JR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "JR not implemented" ); }
  u8 LD( const InstDetails&, u8, u8 ); // { throw std::runtime_error( "LD not implemented" ); }
  u8 LDH( const InstDetails&, u8, u8 ) { throw std::runtime_error( "LDH not implemented" ); }
  u8 NOP( const InstDetails&, u8, u8 ); // { throw std::runtime_error( "NOP not implemented" ); }
  u8 OR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "OR not implemented" ); }
  u8 POP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "POP not implemented" ); }
  u8 PREFIX( const InstDetails&, u8, u8 ) { throw std::runtime_error( "PREFIX not implemented" ); }
  u8 PUSH( const InstDetails&, u8, u8 ) { throw std::runtime_error( "PUSH not implemented" ); }
  u8 RES( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RES not implemented" ); }
  u8 RET( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RET not implemented" ); }
  u8 RETI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RETI not implemented" ); }
  u8 RL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RL not implemented" ); }
  u8 RLA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RLA not implemented" ); }
  u8 RLC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RLC not implemented" ); }
  u8 RLCA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RLCA not implemented" ); }
  u8 RR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RR not implemented" ); }
  u8 RRA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RRA not implemented" ); }
  u8 RRC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RRC not implemented" ); }
  u8 RRCA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RRCA not implemented" ); }
  u8 RST( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RST not implemented" ); }
  u8 SBC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SBC not implemented" ); }
  u8 SCF( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SCF not implemented" ); }
  u8 SET( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SET not implemented" ); }
  u8 SLA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SLA not implemented" ); }
  u8 SRA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SRA not implemented" ); }
  u8 SRL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SRL not implemented" ); }
  u8 STOP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "STOP not implemented" ); }
  u8 SUB( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SUB not implemented" ); }
  u8 SWAP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SWAP not implemented" ); }
  u8 XOR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "XOR not implemented" ); }

  void debug( const InstDetails&, u8, u8 );
private:

  u8 Registers::*pr8[ 8 ] {
    &Registers::B,
    &Registers::C,
    &Registers::D,
    &Registers::E,
    &Registers::H,
    &Registers::L,
    &Registers::F,  // this is actually the read memory at HL which is not an 8-bit register
    &Registers::A,
  };

  u16 Registers::*p16[ 4 ] {
    &Registers::BC,
    &Registers::DE,
    &Registers::HL,
    &Registers::SP,  // &Registers::AF,  // changed for the LD insruction
  };

  std::unordered_map< std::string, int > flagOffsets {
    { "Z", 7 },
    { "N", 6 },
    { "H", 5 },
    { "C", 4 },
  };

private:
  Bus* bus;

    InstDetails instrs[ 512 ] {
#include "../src/_insr_details.hh"
    };
};

#endif
