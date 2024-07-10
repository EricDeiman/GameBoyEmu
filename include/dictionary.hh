#ifndef __dictionary_hh__
#define __dictionary_hh__

#include <string>
#include <unordered_map>

template <typename K = std::string, typename V = std::string>
using dictionary = std::unordered_map< K, V >;

#endif
