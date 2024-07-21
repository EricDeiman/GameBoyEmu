
#include "../include/bus.hh"
#include "../include/cpu.hh"
#include "../include/ram.hh"
#include "../include/serial.hh"
#include "../include/timer.hh"

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

void
Bus::initialize( CPU* cpu, RAM* ram, Timer* timer, Serial* serial ) {
  this->cpu = cpu;
  this->ram = ram;
  this->timer = timer;
  this->serial = serial;
}

void
Bus::doIO( u16 address, u8 data ) {
  if( address < 0xff00 || 0xff7f < address ) {
    char buffer[ 1024 ] = { 0 };
    sprintf( buffer, "Attempt to do IO at address 0x%04x which is outside of IO map; doing nothing.", address );
    _log->Write( Log::warn, buffer );
  }
  else {
    switch( address ) {
    case DIV:
      // Any write to the divider address causes the value to be reset
      data = 0;
      ram->write( address, data );
      break;

    case TAC:
      timer->setTAC( data );
      ram->write( address, data );
      break;

    case IF:
      ram->write( address, data );
      break;

    case SB:
      ram->write( address, data );
      break;

    case SC:
      ram->write( address, data );
      serial->write();
      break;

    case LY:{
      // TODO: Fix me, should this work?  For debugging cpu_instrs.gb
      char buffer[ 1024 ] = { 0 };
      sprintf( buffer, "Write to LCD LY port with data 0x%02x from address 0x%04x"
               " not properly implemented yet",
               data, cpu->addrCurrentInstr );
      _log->Write( Log::warn, buffer );
      ram->write( address, data );
      }
      break;

    case LCDC:{
      // TODO: Fix me, should this work?  For debugging cpu_instrs.gb
      char buffer[1024] = {0};
      sprintf( buffer, "Write to LCD Control port with data 0x%02x from address 0x%04x"
               " not properly implemented yet",
               data, cpu->addrCurrentInstr );
      _log->Write(Log::warn, buffer);
      ram->write( address, data );
      }
      break;

    case 0xff4f:
    case 0xff68:
    case 0xff69: {
      // Not sure why the cpu_instr.gb test rom is writing here.  This port has something
      // to do with Color Game Boy BG palettes index.
      char buffer[ 1024 ] = { 0 };
      sprintf( buffer, "Write to CGB port 0x%04x with data 0x%02x from address 0x%04x. "
               "Why?", address, data, cpu->addrCurrentInstr );
      _log->Write( Log::warn, buffer );
      ram->write( address, data );
      }
      break;

    default: {
      if( IOAddress::SOUND_START <= address && address <= IOAddress::SOUND_END ) {
        char buffer[ 1024 ] = { 0 };
        sprintf( buffer, "Sound IO port 0x%04x not implemented yet, attempt to"
                 " write 0x%02x, from address 0x%04x",
                 address, data, cpu->addrCurrentInstr );
        _log->Write( Log::warn, buffer );
      }
      else {
        char buffer[ 1024 ] = { 0 };
        sprintf( buffer, "Bus::doIO to address 0x%04x not implemented from address 0x%04x",
                 address, cpu->addrCurrentInstr );
        throw std::runtime_error( buffer );
      }
    }
    }
  }
}

u8
Bus::read( u16 address ) {
  return ram->read8( address );
}

std::string
Bus::hexDump( u16 start, u16 count ) {
  return ram->hexDump( start, count );
}

void
Bus::write(u16 address, u8 data ){

  if( address == Bus::IOAddress::DIV ) {
    // Any write to the DIV io port set the port to zero
    data = 0;
  }

  if( 0xff00 <= address && address <= 0xff7f ) {
    doIO( address, data );
  }
  else {
    ram->write( address, data );
  }
}

Timer *
Bus::getTimer() {
  return timer;
}
