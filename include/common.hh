#ifndef __common_hh__
#define __common_hh__

#include <cinttypes>
#include <iomanip>
#include <string>
#include <unordered_map>

#include "dictionary.hh"

using u8 = std::uint8_t;
using i8 = std::int8_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;

#include "config.hh"
#include "log.hh"

extern Config *conf;
extern Log *_log;

// from https://cplusplus.com/forum/beginner/75750/
struct setHex {
  explicit constexpr setHex(u8 width) : width(width) {}
  u8 width;

  inline friend std::ostream &operator<<(std::ostream &os,
                                         const setHex &manip) {
    return os << std::hex << std::setw(manip.width) << std::setfill('0');
  }
};

#endif
