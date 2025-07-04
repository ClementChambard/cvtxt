#ifndef RENDERBATCH_HPP
#define RENDERBATCH_HPP

#include "../defines.hpp"
#include "color.hpp"
#include "renderlist.hpp"
#include <vector>

struct RenderBatch {
  struct Vertex {
    f32 x, y;
    Color c;
    // texture => for text
  };
  using Index = u32;
  std::vector<Vertex> vertices;
  std::vector<Index> indices;
  u32 vbo, ibo, vao;
  RenderBatch(RenderBatch const &) = delete;
  RenderBatch &operator=(RenderBatch const &) = delete;
  RenderBatch();
  ~RenderBatch();
  RenderBatch(RenderBatch &&);
  RenderBatch &operator=(RenderBatch &&);
  void rect(RenderCmd const &c);
  void end();
  void use();
  void render();
};

#endif // !RENDERBATCH_HPP
