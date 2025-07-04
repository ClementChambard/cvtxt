#ifndef BASESHADER_HPP
#define BASESHADER_HPP

#include "../defines.hpp"

struct BaseShader {
  u32 program;
  BaseShader(BaseShader const &) = delete;
  BaseShader &operator=(BaseShader const &) = delete;
  BaseShader(BaseShader &&);
  BaseShader &operator=(BaseShader &&);
  BaseShader();
  ~BaseShader();
  void use();
};

#endif // !BASESHADER_HPP
