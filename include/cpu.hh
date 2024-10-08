#ifndef __cpu_hh__
#define __cpu_hh_

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "common.hh"
#include "dictionary.hh"

class Bus;

class CPU {
public:
  CPU();

  void initialize( Bus* );

  void _clock();

  u16 addrCurrentInstr = 0;
  uint64_t ticks = 0;
  uint64_t waitUntilTicks = 0;
  bool interruptsEnabled = false;

  struct Registers {
    union {
      struct {
        u8 C;
        u8 B;
      };
      u16 BC;
    };

    union {
      struct {
        u8 E;
        u8 D;
      };
      u16 DE;
    };

    union {
      struct {
        u8 L;
        u8 H;
      };
      u16 HL;
    };

    union {
      struct {
        u8 F;
        u8 A;
      };
      u16 AF;
    };

    u16 SP;
    u16 PC;
  } regs;

  enum Interrupt {
    VBlank = 0b0000'0001,
    LCD    = 0b0000'0010,
    Timer  = 0b0000'0100,
    Serial = 0b0000'1000,
    Joypad = 0b0001'0000,
    Stat   = 0b0010'0000  // Not an interrupt flag, but still an interrupt
  };

  void triggerInterrupt( Interrupt );
  void triggerStatInterrupt();

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

  u8 ADC( const InstDetails&, u8, u8 );
  u8 ADD( const InstDetails&, u8, u8 );
  u8 AND( const InstDetails&, u8, u8 );
  u8 BIT( const InstDetails&, u8, u8 ) { throw std::runtime_error( "BIT not implemented" ); }
  u8 CALL( const InstDetails&, u8, u8 );
  u8 CCF( const InstDetails&, u8, u8 ) { throw std::runtime_error( "CCF not implemented" ); }
  u8 CP( const InstDetails&, u8, u8 );
  u8 CPL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "CPL not implemented" ); }
  u8 DAA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "DAA not implemented" ); }
  u8 DBG( const InstDetails&, u8, u8 );
  u8 DEC( const InstDetails&, u8, u8 );
  u8 DI( const InstDetails&, u8, u8 );
  u8 EI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "EI not implemented" ); }
  u8 HALT( const InstDetails&, u8, u8 ) { throw std::runtime_error( "HALT not implemented" ); }
  u8 ILL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "ILL not implemented" ); }
  u8 INC( const InstDetails&, u8, u8 );
  u8 JP( const InstDetails&, u8, u8 );
  u8 JR( const InstDetails&, u8, u8 );
  u8 LD( const InstDetails&, u8, u8 );
  u8 LDH( const InstDetails&, u8, u8 );
  u8 NOP( const InstDetails&, u8, u8 );
  u8 OR( const InstDetails&, u8, u8 );
  u8 POP( const InstDetails&, u8, u8 );
  u8 PREFIX( const InstDetails&, u8, u8 );
  u8 PUSH( const InstDetails&, u8, u8 );
  u8 RES( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RES not implemented" ); }
  u8 RET( const InstDetails&, u8, u8 );
  u8 RETI( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RETI not implemented" ); }
  u8 RL( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RL not implemented" ); }
  u8 RLA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RLA not implemented" ); }
  u8 RLC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RLC not implemented" ); }
  u8 RLCA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RLCA not implemented" ); }
  u8 RR( const InstDetails&, u8, u8 );
  u8 RRA( const InstDetails&, u8, u8 );
  u8 RRC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RRC not implemented" ); }
  u8 RRCA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RRCA not implemented" ); }
  u8 RST( const InstDetails&, u8, u8 ) { throw std::runtime_error( "RST not implemented" ); }
  u8 SBC( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SBC not implemented" ); }
  u8 SCF( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SCF not implemented" ); }
  u8 SET( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SET not implemented" ); }
  u8 SLA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SLA not implemented" ); }
  u8 SRA( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SRA not implemented" ); }
  u8 SRL( const InstDetails&, u8, u8 );
  u8 STOP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "STOP not implemented" ); }
  u8 SUB( const InstDetails&, u8, u8 );
  u8 SWAP( const InstDetails&, u8, u8 ) { throw std::runtime_error( "SWAP not implemented" ); }
  u8 XOR( const InstDetails&, u8, u8 );

  std::string debugSummary( const InstDetails&, u8, u8 );
  std::string debugGameboyDoctor( const InstDetails &, u8, u8 );

  std::string (CPU::*tracer)(const InstDetails &, u8, u8) = &CPU::debugSummary;

  // This method halts everything until it returns
  void debug(const InstDetails &, u8, u8);

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

  u16 Registers::*pr16_1[ 4 ] {
    &Registers::BC,
    &Registers::DE,
    &Registers::HL,
    &Registers::SP
  };

  u16 Registers::*pr16_2[ 4 ]{
      &Registers::BC,
      &Registers::DE,
      &Registers::HL,
      &Registers::AF
  };

  dictionary< std::string, int > flagOffsets {
    { "Z", 7 },
    { "N", 6 },
    { "H", 5 },
    { "C", 4 },
  };

private:
  Bus* bus;
  u8 debugOpcode = 0xd3;
  std::ofstream trace;

  dictionary< Interrupt, u16 > interruptHandler {
    { Interrupt::VBlank, 0x40 },
    { Interrupt::Timer, 0x50 },
    { Interrupt::Serial, 0x58 },
    { Interrupt::Joypad, 0x60 },
    { Interrupt::Stat, 0x48 }
  };

  bool processStatInterrupt = false;

  u8 processingInterrupt = 0;  // Set in the  processInterrupts method, cleared in the RETI method

  void processInterrupts();

  void push( u16 );
  u16 pop();

  bool checkCondCode( u8 );

  dictionary< u16, u8 > breakpoints;

  std::ostream& formatHex( std::ostream&, int );

  void add( int, int, u8& );
  void inc( unsigned, unsigned, u8& );
  u8 rotateRightC( u8 );
  u8 rotateRight( u8 );

  int Zmask = 0b1000'0000;
  int Nmask = 0b0100'0000;
  int Hmask = 0b0010'0000;
  int Cmask = 0b0001'0000;

  u16 prefixForAddress = -1;

  InstDetails ins_decode;
  u8 params[ 2 ] = { 0 };

  void decode();
  void prefixDecode();

  void ( CPU::*decodeHandle )() = &CPU::decode;

  std::vector< void ( CPU::* )() > preExec;

  void Trace();
  void Step();

  bool dbgStep(std::stringstream &);
  bool dbgDump( std::stringstream& );
  bool dbgBreak( std::stringstream& );
  bool dbgContinue( std::stringstream& );
  bool dbgPoke( std::stringstream& );
  bool dbgSetPC( std::stringstream& );

  struct dbgCmd {
    bool ( CPU::*handle )( std::stringstream & );
    std::string desc;
  };

  dictionary< std::string, dbgCmd >
  dbgHandlers = {
    { "step",     { &CPU::dbgStep, "(s)tep" } },
    { "s",        { &CPU::dbgStep, "" } },
    { "dump",     { &CPU::dbgDump, "(d)ump <address>" } },
    { "d",        { &CPU::dbgDump, "" } },
    { "break",    { &CPU::dbgBreak, "(b)reak <address>" } },
    { "b",        { &CPU::dbgBreak, "" } },
    { "continue", { &CPU::dbgContinue, "(c)ontinue" } },
    { "c",        { &CPU::dbgContinue, "" } },
    { "poke",     { &CPU::dbgPoke, "(p)oke <address> <data>" } },
    { "p",        { &CPU::dbgPoke, "" } },
    { "setPC",    { &CPU::dbgSetPC, "setPC <address>" } }
  };

  InstDetails instrs[512]{
#include "../src/_insr_details.hh"
  };
};

#endif
