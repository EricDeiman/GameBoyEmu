#ifndef __bus_h__
#define __bus_h__

#include <string>

#include "common.hh"

class CPU;
class RAM;
class Timer;
struct Serial;

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
// $FF12	  NR12	  Sound channel 1 volume & envelope	R/W	
// $FF13	  NR13	  Sound channel 1 period low	W	
// $FF14	  NR14	  Sound channel 1 period high & control	Mixed	
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
// $FF26	  NR52	  Sound on/off	          Mixed	
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

class Bus {
public:
  enum IOAddress {
    P1JOYP = 0xFF00,
    SB     = 0xFF01,
    SC     = 0xFF02,
    DIV    = 0xFF04,
    TIMA   = 0xFF05,
    TMA    = 0xFF06,
    TAC    = 0xFF07,
    IF     = 0xFF0F,
    NR10   = 0xFF10,
    NR11   = 0xFF11,
    NR12   = 0xFF12,
    NR13   = 0xFF13,
    NR14   = 0xFF14,
    NR21   = 0xFF16,
    NR22   = 0xFF17,
    NR23   = 0xFF18,
    NR24   = 0xFF19,
    NR30   = 0xFF1A,
    NR31   = 0xFF1B,
    NR32   = 0xFF1C,
    NR33   = 0xFF1D,
    NR34   = 0xFF1E,
    NR41   = 0xFF20,
    NR42   = 0xFF21,
    NR43   = 0xFF22,
    NR44   = 0xFF23,
    NR50   = 0xFF24,
    NR51   = 0xFF25,
    NR52   = 0xFF26,
    SOUND_START = NR10,
    SOUND_END = 0xff3f,
    LCDC   = 0xFF40,
    STAT   = 0xFF41,
    SCY    = 0xFF42,
    SCX    = 0xFF43,
    LY     = 0xFF44,
    LYC    = 0xFF45,
    DMA    = 0xFF46,
    BGP    = 0xFF47,
    OBP0   = 0xFF48,
    OBP1   = 0xFF49,
    WY     = 0xFF4A,
    WX     = 0xFF4B,
  };

  void initialize( CPU*, RAM*, Timer*, Serial* );

  u8 read( u16 );
  void write( u16, u8 );
  void dbgWrite( u16, u8 );

  std::string hexDump( u16, u16 );

  void doIO( u16, u8 );

  Timer* getTimer();
  CPU* getCPU();

private:
  CPU* cpu;
  RAM* ram;
  Timer* timer;
  Serial* serial;
};

#endif
