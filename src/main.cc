
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "../include/board.hh"
#include "../include/config.hh"
#include "../include/log.hh"


Config *conf;
Log *_log;

struct DefaultDetails {
  std::string defaultValue;
  std::string description;
};

dictionary< std::string, DefaultDetails> CommandLineDefaults {
  { "-C", { "GameBoyEmu.conf", "Configuration file for the emulator" } },
  { "-L", { "GameBoyEmu.log", "Log file for the emulator" } },
};

std::string
generateUsageMessage() {
  std::vector< std::string > keys;
  std::stringstream rtn;

  for( auto& i : CommandLineDefaults ) {
    keys.push_back( i.first );
  }

  std::sort( keys.begin(), keys.end() );

  for( auto& k : keys ) {
    rtn << k << " " << CommandLineDefaults[ k ].description;
    if( CommandLineDefaults[ k ].defaultValue.length() > 0 ) {
      rtn << " " << CommandLineDefaults[ k ].defaultValue;
    }
    rtn << std::endl;
  }

  return rtn.str();
}

dictionary<>
getCommandLineDefaults() {
  dictionary<> cmdl;

  cmdl[ "-C" ] = CommandLineDefaults[ "-C" ].defaultValue;
  cmdl[ "-L" ] = CommandLineDefaults[ "-L" ].defaultValue;

  return cmdl;
}

void
processCommandLine( int argc, char** argv, dictionary<>& args ) {
  for( int i = 1; i < argc; i++ ) {
    char* arg = argv[ i ];

    if( arg[ 0 ] == '-' && ( i + 1 < argc ) ) { // process a switch
      std::string key{ arg };

      std::string value{ argv[ i + 1 ] };
      args[ key ] = value;
      i++;
    }
  }
}

int
main( int argc, char** argv ) {

  auto cmdl{ getCommandLineDefaults() };

  processCommandLine( argc, argv, cmdl );

  Config _conf{ cmdl };

  conf = &_conf;

  // get the log file name
  auto fn{ cmdl[ "-L" ] };  // by default, this exists
  Log log{ fn };
  _log = &log;

  log.Write( Log::info, "=== [ GameBoyEmu started ] ===" );
  log.Write( Log::info, "Command line switches:" );

  for( auto& [ key, value ] : cmdl ) {
    log.Write( Log::info, "   " + key + " " + value );
  }

  log.Write( Log::info, "Configuration file entries:" );
  log.Write( Log::info, "   Config file name: " + _conf.GetFileName() );
  for( auto& key : _conf.GetKeys() ) {
    log.Write( Log::info, "   " + key + " = " + _conf.GetValue( key ) );
  }

  Board board;
  board._clock();

  log.Write( Log::info, "GameBoyEmu ended" );

  return 0;
}
