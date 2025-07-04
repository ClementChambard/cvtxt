#include "renderbatch.hpp"

#include <GL/glew.h>
#include <cassert>
#include <cmath>
#include <numbers>

RenderBatch::RenderBatch() {
  glCreateBuffers(1, &vbo);
  glCreateBuffers(1, &ibo);
  glCreateVertexArrays(1, &vao);
  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
  glVertexArrayElementBuffer(vao, ibo);
  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, false, 0);
  glVertexArrayAttribFormat(vao, 1, 4, GL_UNSIGNED_BYTE, true, 2 * sizeof(f32));
}
RenderBatch::~RenderBatch() {
  if (vao)
    glDeleteVertexArrays(1, &vao);
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (ibo)
    glDeleteBuffers(1, &ibo);
}
RenderBatch::RenderBatch(RenderBatch &&o) {
  vbo = o.vbo;
  ibo = o.ibo;
  vao = o.vao;
  o.vao = o.ibo = o.vbo = 0;
  vertices = std::move(o.vertices);
  indices = std::move(o.indices);
}
RenderBatch &RenderBatch::operator=(RenderBatch &&o) {
  vbo = o.vbo;
  ibo = o.ibo;
  vao = o.vao;
  o.vao = o.ibo = o.vbo = 0;
  vertices = std::move(o.vertices);
  indices = std::move(o.indices);
  return *this;
}

void unstrip_indices(RenderBatch &b, std::vector<RenderBatch::Index> &&strip) {
  if (strip.size() == 0)
    return;
  assert(strip.size() >= 3);
  u32 last1 = strip[0];
  u32 last2 = strip[1];
  bool pair = true;
  for (u64 i = 2; i < strip.size(); i++) {
    u32 cur = strip[i];
    b.indices.push_back(last1);
    if (pair) {
      b.indices.push_back(last2);
      b.indices.push_back(cur);
    } else {
      b.indices.push_back(cur);
      b.indices.push_back(last2);
    }
    last1 = last2;
    last2 = cur;
    pair = !pair;
  }
}

void RenderBatch::rect(RenderCmd const &c) {
  auto index0 = vertices.size();
  if (c.r == 0) {
    vertices.push_back({c.x, c.y, c.c});
    vertices.push_back({c.x + c.w, c.y, c.c});
    vertices.push_back({c.x + c.w, c.y + c.h, c.c});
    vertices.push_back({c.x, c.y + c.h, c.c});
    indices.push_back(index0 + 0);
    indices.push_back(index0 + 1);
    indices.push_back(index0 + 2);
    indices.push_back(index0 + 0);
    indices.push_back(index0 + 2);
    indices.push_back(index0 + 3);
    return;
  }
  f32 x1 = c.x + c.r;
  f32 x2 = c.x + c.w - c.r;
  f32 y1 = c.y + c.r;
  f32 y2 = c.y + c.h - c.r;
  int n_point_needed = std::numbers::pi / (4.0f * std::acos(1 - 0.33 / c.r));
  vertices.push_back({c.x, y1, c.c});
  vertices.push_back({c.x + c.w, y1, c.c});
  vertices.push_back({c.x + c.w, y2, c.c});
  vertices.push_back({c.x, y2, c.c});
  auto index1 = vertices.size();
  vertices.push_back({x1, c.y, c.c});
  vertices.push_back({x2, c.y, c.c});
  vertices.push_back({x2, c.y + c.h, c.c});
  vertices.push_back({x1, c.y + c.h, c.c});
  auto index2 = vertices.size();
  vertices.resize(vertices.size() + 4 * n_point_needed);
  auto index3 = index2 + n_point_needed;
  auto index4 = index3 + n_point_needed;
  auto index5 = index4 + n_point_needed;
  f32 incr = (std::numbers::pi / 2.f) / (n_point_needed + 1.f);
  auto pt_x = [&](f32 x, f32 a) { return x + std::cos(a) * c.r; };
  auto pt_y = [&](f32 y, f32 a) { return y - std::sin(a) * c.r; };
  for (int i = 0; i < n_point_needed; i++) {
    f32 pos = incr * (1 + i);
    f32 a_tl = std::numbers::pi - pos;
    f32 a_bl = std::numbers::pi + pos;
    f32 a_tr = std::numbers::pi / 2.f - pos;
    f32 a_br = -std::numbers::pi / 2.f + pos;
    vertices[index2 + i] = {pt_x(x1, a_tl), pt_y(y1, a_tl), c.c};
    vertices[index3 + i] = {pt_x(x2, a_tr), pt_y(y1, a_tr), c.c};
    vertices[index4 + i] = {pt_x(x2, a_br), pt_y(y2, a_br), c.c};
    vertices[index5 + i] = {pt_x(x1, a_bl), pt_y(y2, a_bl), c.c};
  }
  std::vector<Index> strip;
  strip.push_back(index0);
  strip.push_back(index0 + 3);
  for (int i = 0; i < n_point_needed; i++) {
    strip.push_back(index2 + i);
    strip.push_back(index5 + i);
  }
  strip.push_back(index1 + 0);
  strip.push_back(index1 + 3);
  strip.push_back(index1 + 1);
  strip.push_back(index1 + 2);
  for (int i = 0; i < n_point_needed; i++) {
    strip.push_back(index3 + i);
    strip.push_back(index4 + i);
  }
  strip.push_back(index0 + 1);
  strip.push_back(index0 + 2);
  unstrip_indices(*this, std::move(strip));
}
void RenderBatch::end() {
  glNamedBufferData(vbo, sizeof(Vertex) * vertices.size(), vertices.data(),
                    GL_DYNAMIC_DRAW);
  glNamedBufferData(ibo, sizeof(Index) * indices.size(), indices.data(),
                    GL_DYNAMIC_DRAW);
}
void RenderBatch::render() {
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}
void RenderBatch::use() { glBindVertexArray(vao); }
