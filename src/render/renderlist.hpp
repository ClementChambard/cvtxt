#ifndef RENDERLIST_HPP
#define RENDERLIST_HPP

#include "color.hpp"
#include <vector>

struct RenderCmd {
  // only boxes for now
  f32 x, y, w, h, r;
  Color c;
};

using RenderList = std::vector<RenderCmd>;

#endif // !RENDERLIST_HPP
