#ifndef __log_hh__
#define __log_hh__

#include <fstream>
#include <string>

class Log {
public:
  enum LogType {
    info,
    warn,
    error,
    debug
  };

  Log( std::string fileName );
  ~Log();

  void
  Write( LogType, std::string );

  std::string
  toString( LogType );

private:
  std::ofstream os;

};

#endif
