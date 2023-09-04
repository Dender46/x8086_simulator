#pragma once
#include <cstdint>
#include <numeric>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef int8_t      s8;
typedef int16_t     s16;

#define u8_max      std::numeric_limits<u8>::max()
#define u16_max     std::numeric_limits<u16>::max()
#define s8_max      std::numeric_limits<s8>::max()
#define s16_max     std::numeric_limits<s16>::max()
