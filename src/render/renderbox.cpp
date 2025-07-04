#include "renderbox.hpp"

#include "../file/filedata.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

Value get_prop_w_style(LayoutElem const &elt, Style const *style,
                       char const *name, Value default_val = 0.f) {
  if (style) {
    auto it = style->values.find(name);
    if (it != style->values.end())
      return it->second;
  }
  return elt.get_prop(name, default_val);
}

#define RESOLVE resolve_units(cv.width, cv.height)
void assign_inset(Inset &toassign, std::string name, LayoutElem const &elt,
                  Style const *style, CV const &cv) {
  toassign.l = toassign.r = toassign.b = toassign.t =
      get_prop_w_style(elt, style, name.c_str()).RESOLVE;
  toassign.l = toassign.r =
      get_prop_w_style(elt, style, (name + "_x").c_str(), toassign.l).RESOLVE;
  toassign.t = toassign.b =
      get_prop_w_style(elt, style, (name + "_y").c_str(), toassign.t).RESOLVE;
  toassign.t =
      get_prop_w_style(elt, style, (name + "_t").c_str(), toassign.t).RESOLVE;
  toassign.b =
      get_prop_w_style(elt, style, (name + "_b").c_str(), toassign.b).RESOLVE;
  toassign.l =
      get_prop_w_style(elt, style, (name + "_l").c_str(), toassign.l).RESOLVE;
  toassign.r =
      get_prop_w_style(elt, style, (name + "_r").c_str(), toassign.r).RESOLVE;
}

void assign_props(RenderBox &rb, LayoutElem const &elt, CV const &cv) {
  Style const *style = nullptr;
  auto style_id = elt.get_prop("style", Value(Value::STYLE, i32(-1)));
  assert(style_id.kind == Value::STYLE);
  if (style_id.val_int != -1) {
    style = &cv.style[style_id.val_int];
  }

  rb.width = get_prop_w_style(elt, style, "w", INFINITY).RESOLVE;
  rb.height = get_prop_w_style(elt, style, "h", INFINITY).RESOLVE;
  rb.gap = get_prop_w_style(elt, style, "gap").RESOLVE;
  assign_inset(rb.padding, "padding", elt, style, cv);
  assign_inset(rb.margin, "margin", elt, style, cv);
  rb.background_color =
      get_prop_w_style(elt, style, "background_color", Value(Value::COLOR, 0));
  rb.corner_radius = get_prop_w_style(elt, style, "corner_radius").RESOLVE;
}

RenderBox::RenderBox(LayoutElem const &elt, CV const &cv) {
  for (auto const &c : elt.children) {
    children.emplace_back(c, cv);
  }
  if (elt.kind == "layers") {
    children_mode = LAYER;
    assign_props(*this, elt, cv);
  } else if (elt.kind == "box") {
    children_mode = UNIQUE;
    assign_props(*this, elt, cv);
    assert(children.size() <= 1);
  } else if (elt.kind == "column") {
    children_mode = COLUMN;
    assign_props(*this, elt, cv);
  } else if (elt.kind == "row") {
    children_mode = ROW;
    assign_props(*this, elt, cv);
  } else if (elt.kind == "hsplit") {
    children_mode = ROW;
    assign_props(*this, elt, cv);
    assert(children.size() == 2);
    auto loc_prop = elt.get_prop("loc", Value(Value::PC, 50.f));
    assert(loc_prop.kind == Value::PC);
    children[0].width = loc_prop;
    children[1].width = Value(Value::PC, 100.f - loc_prop.val);
  } else if (elt.kind == "vsplit") {
    children_mode = COLUMN;
    assign_props(*this, elt, cv);
    assert(children.size() == 2);
    auto loc_prop = elt.get_prop("loc", Value(Value::PC, 50.f));
    assert(loc_prop.kind == Value::PC);
    children[0].height = loc_prop;
    children[1].height = Value(Value::PC, 100.f - loc_prop.val);
  }
}

void RenderBox::render(f32 x, f32 y, f32 w, f32 h, RenderList &list) const {
  if (background_color.a != 0) {
    // render a rectangle:
    RenderCmd cmd;
    cmd.x = x + margin.l.get_f32(w);
    cmd.w = w - margin.l.get_f32(w) - margin.r.get_f32(w);
    cmd.y = y + margin.t.get_f32(h);
    cmd.h = h - margin.t.get_f32(h) - margin.b.get_f32(h);
    cmd.r = corner_radius.get_f32(std::min(w, h) / 2.f);
    cmd.c = background_color;
    list.push_back(cmd);
  }
  f32 new_x = x + margin.l.get_f32(w) + padding.l.get_f32(w);
  f32 new_y = y + margin.t.get_f32(h) + padding.t.get_f32(h);
  f32 new_w = w - (margin.l.get_f32(w) + padding.l.get_f32(w) +
                   margin.r.get_f32(w) + padding.r.get_f32(w));
  f32 new_h = h - (margin.t.get_f32(h) + padding.t.get_f32(h) +
                   margin.b.get_f32(h) + padding.b.get_f32(h));
  if (children_mode == LAYER) {
    for (auto const &c : children) {
      auto c_w = new_w, c_h = new_h;
      if (c.width.val != INFINITY)
        c_w = c.width.get_f32(c_w);
      if (c.height.val != INFINITY)
        c_h = c.height.get_f32(c_h);
      c.render(new_x, new_y, c_w, c_h, list);
    }
  } else if (children_mode == UNIQUE) {
    if (children.size() > 0) {
      auto const &c = children[0];
      if (c.width.val != INFINITY)
        new_w = c.width.get_f32(new_w);
      if (c.height.val != INFINITY)
        new_h = c.height.get_f32(new_h);
      c.render(new_x, new_y, new_w, new_h, list);
    }
  } else if (children_mode == COLUMN) {
    f32 rem_h = new_h;
    u32 rem_h_cnt = 0;
    auto gap_val = gap.get_f32(h);
    rem_h -= gap_val * std::max(0.f, f32(children.size()) - 1.f);
    for (auto const &c : children) {
      if (c.height.val != INFINITY) {
        rem_h -= c.height.get_f32(h);
      } else {
        rem_h_cnt++;
      }
    }
    if (rem_h_cnt == 0) {
      for (auto const &c : children) {
        f32 c_h = c.height.get_f32(h);
        c.render(new_x, new_y, new_w, c_h, list);
        new_y += c_h + gap_val;
      }
    } else if (rem_h <= 0.f) {
      for (auto const &c : children) {
        f32 c_h = 0;
        if (c.height.val != INFINITY) {
          c_h = c.height.get_f32(h);
          c.render(new_x, new_y, new_w, c_h, list);
        }
        new_y += c_h + gap_val;
      }
    } else {
      rem_h = rem_h / f32(rem_h_cnt);
      for (auto const &c : children) {
        f32 c_h = rem_h;
        if (c.height.val != INFINITY) {
          c_h = c.height.get_f32(h);
        }
        c.render(new_x, new_y, new_w, c_h, list);
        new_y += c_h + gap_val;
      }
    }
  } else if (children_mode == ROW) {
    f32 rem_w = new_w;
    u32 rem_w_cnt = 0;
    auto gap_val = gap.get_f32(w);
    rem_w -= gap_val * std::max(0.f, f32(children.size()) - 1.f);
    for (auto const &c : children) {
      if (c.width.val != INFINITY) {
        rem_w -= c.width.get_f32(w);
      } else {
        rem_w_cnt++;
      }
    }
    if (rem_w_cnt == 0) {
      for (auto const &c : children) {
        f32 c_w = c.width.get_f32(w);
        c.render(new_x, new_y, c_w, new_h, list);
        new_x += c_w + gap_val;
      }
    } else if (rem_w <= 0.f) {
      for (auto const &c : children) {
        f32 c_w = 0;
        if (c.width.val != INFINITY) {
          c_w = c.width.get_f32(w);
          c.render(new_x, new_y, c_w, new_h, list);
        }
        new_x += c_w + gap_val;
      }
    } else {
      rem_w = rem_w / f32(rem_w_cnt);
      for (auto const &c : children) {
        f32 c_w = rem_w;
        if (c.width.val != INFINITY) {
          c_w = c.width.get_f32(w);
        }
        c.render(new_x, new_y, c_w, new_h, list);
        new_x += c_w + gap_val;
      }
    }
  }
}

void RenderBox::needed_size(f32 &w, f32 &h) const {
  w = width.val;
  h = height.val;
}
