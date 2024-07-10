
#include <ctime>
#include <string>

#include "../include/log.hh"

Log::Log(std::string fileName) : os{ fileName, std::ios::app } {}

Log::~Log() { os.close(); }

void
Log::Write( LogType type, std::string message ) {
  auto now = std::time(nullptr);
  std::string nowLocal = std::asctime( std::localtime( &now ) );
  nowLocal = nowLocal.substr( 0, nowLocal.length() - 1 );

  os << nowLocal << "|" << toString( type ) << "|" << message << std::endl;
}

std::string
Log::toString( Log::LogType t ) {
  switch( t ) {
  case info:
    return "INFO";

  case warn:
    return "WARN";

  case error:
    return "ERROR";

  default:
    return "UNKNOWN";
  }
}
