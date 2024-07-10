#ifndef __common_hh__
#define __common_hh__

#include <cinttypes>
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

#endif
