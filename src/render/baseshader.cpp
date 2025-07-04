#include "baseshader.hpp"

#include <GL/glew.h>

BaseShader::BaseShader(BaseShader &&o) {
  program = o.program;
  o.program = 0;
}
BaseShader &BaseShader::operator=(BaseShader &&o) {
  program = o.program;
  o.program = 0;
  return *this;
}
BaseShader::~BaseShader() {
  if (program)
    glDeleteProgram(program);
}
void BaseShader::use() { glUseProgram(program); }

char const *VTX_SHADER_SOURCE = R"(#version 460
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec4 in_color;

layout (location = 0) out vec4 frag_color;

layout (location = 0) uniform vec2 screen_size;

void main() {
  vec2 pos = (in_position / screen_size) * 2.0 - 1.0;
  gl_Position = vec4(pos.x, -pos.y, 0.0, 1.0);
  frag_color = in_color;
}
)";
char const *FRG_SHADER_SOURCE = R"(#version 460
layout (location = 0) in vec4 frag_color;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = frag_color;
}
)";

BaseShader::BaseShader() {
  auto vtx = glCreateShader(GL_VERTEX_SHADER);
  auto frg = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vtx, 1, &VTX_SHADER_SOURCE, nullptr);
  glShaderSource(frg, 1, &FRG_SHADER_SOURCE, nullptr);
  glCompileShader(vtx);
  glCompileShader(frg);
  program = glCreateProgram();
  glAttachShader(program, vtx);
  glAttachShader(program, frg);
  glLinkProgram(program);
  glValidateProgram(program);
  glDetachShader(program, vtx);
  glDetachShader(program, frg);
  glDeleteShader(vtx);
  glDeleteShader(frg);
}
