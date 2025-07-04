#ifndef FILEDATA_HPP
#define FILEDATA_HPP

#include "../defines.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Value { // TODO:
  enum Kind {
    PC,
    VW,
    VH,
    NO_UNIT,
    STYLE,
    COLOR,
  } kind;
  union {
    f32 val;
    i32 val_int;
  };
  Value() = default;
  Value(Kind k, f32 val) : kind(k), val(val) {}
  Value(Kind k, i32 val) : kind(k), val_int(val) {}
  Value(f32 val) : kind(NO_UNIT), val(val) {}
  Value &resolve_units(f32 vw, f32 vh);
  f32 get_f32(f32 pc_mult) const;
};

struct LayoutElem {
  std::string kind;
  std::optional<std::string> name;
  std::unordered_map<std::string, Value> props;
  std::vector<LayoutElem> children;
  Value get_prop(char const *name, Value default_val = 0.f) const;
};

struct Layout {
  std::string name;
  LayoutElem root;
};

struct Variable {
  std::string name;
  Value val;
};

struct Style {
  std::unordered_map<std::string, Value> values;
};

struct CV {
  f32 width, height;
  std::vector<Layout> layout;
  std::vector<Style> style;
  std::vector<Variable> variables;
};

#endif // !FILEDATA_HPP
