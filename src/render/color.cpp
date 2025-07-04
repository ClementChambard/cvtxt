#include "color.hpp"
#include "../file/filedata.hpp"
#include <cassert>

Color::Color(u32 i) {
  a = (i >> 0x00) & 0xff;
  b = (i >> 0x08) & 0xff;
  g = (i >> 0x10) & 0xff;
  r = (i >> 0x18) & 0xff;
}

Color::Color(Value v) : Color(v.val_int) { assert(v.kind == Value::COLOR); }

Color::operator u32() const {
  return u32(a) | (u32(b) << 8) | (u32(g) << 0x10) | (u32(r) << 0x18);
}
