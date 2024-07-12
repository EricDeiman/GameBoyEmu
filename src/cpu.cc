#include <vector>

#include <iomanip>
#include <iostream>
#include <ios>

#include "../include/cpu.hh"
#include "../include/common.hh"

void
CPU::_clock() {

  // Read the instruction at PC
  u8 ins1 = bus->read( regs.PC );
  u8 ins2 = bus->read( regs.PC + 1 );

  std::cout << "The first two bytes are " <<
    std::hex << std::setfill( '0' ) << std::setw( 4 ) <<
    ( regs.PC & 0xffff ) << ": " << std::setw( 2 ) << ( ins1 & 0xff ) << " " <<
    ( ins2 & 0xff ) << std::endl;

}

void
CPU::ADC(const InstDetails &instr, u8 parm1, u8 parm2) {
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
}
