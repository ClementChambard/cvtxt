#include "filedata.hpp"
#include <cassert>
#include <unordered_map>

Value LayoutElem::get_prop(char const *name, Value default_val) const {
  if (auto it = props.find(name); it != props.end()) {
    return it->second;
  }
  return default_val;
}

Value &Value::resolve_units(f32 vw, f32 vh) {
  if (kind == Value::VW) {
    val = val / 100.f * vw;
    kind = NO_UNIT;
  }
  if (kind == Value::VH) {
    val = val / 100.f * vh;
    kind = NO_UNIT;
  }
  return *this;
}

f32 Value::get_f32(f32 pc_mult) const {
  if (kind == Value::PC) {
    return val / 100.f * pc_mult;
  }
  assert(kind == Value::NO_UNIT);
  return val;
}

std::string error_message; // TODO: put message in box
bool parsing_had_error;

CV get_error_document() {
  std::unordered_map<std::string, Value> props;
  props["background_color"] = Value(Value::COLOR, i32(0xff0000ff));
  props["padding"] = Value(Value::VW, 10);
  props["margin"] = Value(Value::VW, 10);
  props["corner_radius"] = Value(Value::VW, 3);
  return CV{
      .width = 1,
      .height = 1,
      .layout = {Layout{"%root%",
                        LayoutElem{
                            .kind = "box",
                            .name = "",
                            .props = props,
                            .children = {},
                        }}},
      .style = {},
      .variables = {},
  };
}
