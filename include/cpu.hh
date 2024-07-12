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

  struct InstDetails;

  void ADC( const InstDetails&, u8, u8 ); // { throw std::runtime_error( "not implemented" ); }
  void ADD( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void AND( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void BIT( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void CALL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void CCF( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void CP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void CPL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void DAA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void DBG( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void DEC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void DI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void EI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void HALT( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void ILL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void INC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void JP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void JR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void LD( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void LDH( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void NOP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void OR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void POP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void PREFIX( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void PUSH( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RES( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RET( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RETI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RLA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RLC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RLCA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RRA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RRC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RRCA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void RST( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SBC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SCF( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SET( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SLA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SRA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SRL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void STOP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SUB( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void SWAP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }
  void XOR( const InstDetails&, u8, u8 ) { throw std::runtime_error( "not implemented" ); }

  void _clock();

private:

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
    &Registers::AF,
  };

  std::unordered_map< std::string, int > flagOffsets {
    { "Z", 7 },
    { "N", 6 },
    { "H", 5 },
    { "C", 4 },
  };

  enum AddressingModes {
    am_ins, // instruction encoding has all the necessary info
    am_d8,  // the argument is 1-byte data
    am_d16, // the argument is 2-byte data
    am_r8,  // the argument is an 1-byte register
    am_a16, // the argument is a 2-byte address
    am_ia8, // the argument is a 1-byte indirect address
    am_ia16 // the argument is a 2-byte indirect address
  };

public:
    struct InstDetails {
      u16 binary;         // actual op-code bits
      std::string desc;         // description of the opcode from docs
      void ( CPU::*impl )( const InstDetails &, u8, u8 ); // pointer to function to interpret the op-code
      AddressingModes am; // addressing mode of opcode
      int bytes; // number of bytes used encode entire instruction and arguments
      int cycles1; // number of cycles instruction uses
      int cycles2; // alternate number of cycles instruction takes
      char Z;      // zero flag
      char N;      // subtract flag
      char H;      // half carry flag
      char C;      // carry flag
    };

private:
  Bus* bus;

    InstDetails instrs[ 512 ] {
#include "../src/_insr_details.hh"
    };
};

#endif
