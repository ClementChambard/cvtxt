#ifndef RENDERBOX_HPP
#define RENDERBOX_HPP

#include "../file/filedata.hpp"
#include "color.hpp"
#include "renderlist.hpp"
#include <vector>

struct LayoutElem;
struct CV;

struct TextStyle {
  // TODO:
};

struct Inset {
  Value l = 0.f, r = 0.f, t = 0.f, b = 0.f;
};

struct ContentElem;

struct RenderBox {
  enum ChildrenMode {
    UNIQUE, // element put in top-left of container
    COLUMN, // elements disposed in a dynamically calculated column
    ROW,    // elements disposed in a dynamically calculated row
    LAYER,  // same as unique but can have multiple children
  } children_mode = UNIQUE;
  std::vector<RenderBox> children = {};
  std::vector<TextStyle> text_styles = {};
  Value width = -1.0f;
  Value height = -1.0f;
  Value gap = 0.0f;
  Inset margin = {};
  Inset padding = {};
  Color background_color;
  ContentElem *content = nullptr;
  Value corner_radius = 0.0f;

  RenderBox() = default;
  RenderBox(LayoutElem const &, CV const &);

  void needed_size(f32 &w, f32 &h) const;

  void render(f32 x, f32 y, f32 w, f32 h, RenderList &list) const;
};

#endif // !RENDERBOX_HPP
