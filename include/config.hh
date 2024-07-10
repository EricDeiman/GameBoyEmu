#ifndef __config_hh__
#define __config_hh__

#include <string>
#include <unordered_map>
#include <vector>

#include "dictionary.hh"


// TODO: Convert to sublass of unordered map
// TODO: Make # the comment to eol token in the config file
class Config {
public:
  Config( dictionary<>& cmdl );

  std::vector< std::string >
  GetKeys();

  std::string
  GetValue( std::string );

  const std::string&
  GetFileName();

private:
  dictionary<> _config{};
  std::string fileName;

  std::vector<std::string>
  split(const std::string &input,
        const std::string &reText);

  std::string&
  trim( std::string& s );
};

#endif
