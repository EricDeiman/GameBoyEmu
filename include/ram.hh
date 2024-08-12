#ifndef __ram_hh__
#define __ram_hh__

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "common.hh"

#include "mbc.hh"

// TODO:This is where the address map needs to be implemented.

// Game Boy memory map
//  $FFFF 	      Interrupt Enable Flag
//  $FF80-$FFFE 	Zero Page - 127 bytes
//  $FF00-$FF7F 	Hardware I/O Registers
//  $FEA0-$FEFF 	Unusable Memory
//  $FE00-$FE9F 	OAM - Object Attribute Memory
//  $E000-$FDFF 	Echo RAM - Reserved, Do Not Use
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

// Address	Name	  Description	            R/W
// $FF00	  P1/JOYP	Joypad	                Mixed
// $FF01	  SB	    Serial transfer data	  R/W
// $FF02	  SC	    Serial transfer control	R/W	Mixed
// $FF04	  DIV	    Divider register	      R/W
// $FF05	  TIMA	  Timer counter	          R/W
// $FF06	  TMA	    Timer modulo	          R/W
// $FF07	  TAC	    Timer control	          R/W
// $FF0F	  IF	    Interrupt flag	        R/W
// $FF10	  NR10	  Sound channel 1 sweep	  R/W
// $FF11	  NR11	  Sound channel 1 length timer & duty cycle	Mixed
// // $FF12	  NR12	  Sound channel 1 volume & envelope	R/W
// $FF13	  NR13	  Sound channel 1 period low	W
// // $FF14	  NR14	  Sound channel 1 period high & control	Mixed
// $FF16	  NR21	  Sound channel 2 length timer & duty cycle	Mixed
// $FF17	  NR22	  Sound channel 2 volume & envelope	R/W
// $FF18	  NR23	  Sound channel 2 period low	W
// $FF19	  NR24	  Sound channel 2 period high & control	Mixed
// $FF1A	  NR30	  Sound channel 3 DAC enable	R/W
// $FF1B	  NR31	  Sound channel 3 length timer	W
// $FF1C	  NR32	  Sound channel 3 output level	R/W
// $FF1D	  NR33	  Sound channel 3 period low	W
// $FF1E	  NR34	  Sound channel 3 period high & control	Mixed
// $FF20	  NR41	  Sound channel 4 length timer	W
// $FF21	  NR42	  Sound channel 4 volume & envelope	R/W
// $FF22	  NR43	  Sound channel 4 frequency & randomness	R/W
// $FF23	  NR44	  Sound channel 4 control	Mixed
// $FF24	  NR50	  Master volume & VIN panning	R/W
// $FF25	  NR51	  Sound panning	          R/W
// $FF26	  NR52	  Sound on/off	          Mixe
// $FF30-FF3F	Wave RAM	Storage for one of the sound channels’ waveform	R/W
// $FF40	  LCDC	  LCD control	            R/W
// $FF41	  STAT	  LCD status	            Mixed
// $FF42	  SCY	    Viewport Y position	    R/W
// $FF43	  SCX	    Viewport X position	    R/W
// $FF44	  LY	    LCD Y coordinate	      R
// $FF45	  LYC	    LY compare	            R/W
// $FF46	  DMA	    OAM DMA source address & start	R/W
// $FF47	  BGP	    BG palette data	        R/W
// $FF48	  OBP0	  OBJ palette 0 data	    R/W
// $FF49	  OBP1	  OBJ palette 1 data	    R/W
// $FF4A	  WY	    Window Y position	      R/W
// $FF4B	  WX	    Window X position plus 7	R/W

class Bus;

class RAM {
public:

  RAM();

  u8 read8( u16 );
  void write( u16, u8 );
  void dbgWrite( u16, u8 );

  void setBus( Bus* );

  std::string hexDump( u16, u16 );

  void changeBank( u16 );

  enum banks {
    Bank0 = 0x3fff,
    BankN = 0x7fff,
  };

private :
  std::vector< char > _ram;
  std::vector< char > _cart;
  Bus* _bus;

  u16 maxCartRom = 0xfdff;

  std::shared_ptr< MBC >mbc;

  u16 bank = 1;
};

#endif
