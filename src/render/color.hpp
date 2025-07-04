#ifndef COLOR_HPP
#define COLOR_HPP

#include "../defines.hpp"

struct Value;

struct Color {
  u8 r = 0, g = 0, b = 0, a = 0;
  Color() = default;
  Color(u32 i);
  Color(Value v);
  operator u32() const;
};

#endif // !COLOR_HPP
