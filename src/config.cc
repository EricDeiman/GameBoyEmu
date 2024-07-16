
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "../include/config.hh"

Config::Config( dictionary<>& cmdl ) {
  fileName = cmdl[ "-C" ];
  std::ifstream is{ fileName };

  std::string line;
  std::regex rex{ "=" };

  while (std::getline(is, line)) {
    line = line.substr(0, line.find_first_of( '#' ) );
    if( line.find_first_not_of( " \t" ) == std::string::npos ) {
      continue;
    }
    
    std::vector< std::string >res{ split( line, "=" ) };

    _config[ res[ 0 ] ] = res[ 1 ];
  }
}

const std::string&
Config::GetFileName(){
  return fileName;
}
std::vector<std::string>
Config::GetKeys() {
  std::vector< std::string >rtn;

  for( auto& p : _config ) {
    rtn.push_back( p.first );
  }

  return rtn;
}

std::string
Config::GetValue( std::string key ) {
  auto found{ _config.find( key ) };

  if( found != _config.end() ) {
    return found->second;
  }
  else {
    // The key was not found.  What's best to do?
    return "";
  }

}

std::vector<std::string>
Config::split(const std::string &input,
              const std::string &delim) {
  std::string key{ input, 0, input.find_first_of( delim ) };
  std::string value{ input, input.find_first_of( delim ) + 1 };
  return { trim( key ), trim( value ) };
}

std::string&
Config::trim( std::string& s ) {
  const char *t = " \t\n\r\f\v";
  s.erase( 0, s.find_first_not_of( t ) );
  s.erase( s.find_last_not_of( t ) + 1 );
  return s;
}
