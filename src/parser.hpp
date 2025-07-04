#ifndef PARSER_HPP
#define PARSER_HPP

#include "filedata.hpp"
#include "lexer.hpp"

struct Parser {
  Lexer l;
  Token tok;
  Token &next_token();
  CV read_cv_file(char const *filename);
};

#endif // !PARSER_HPP
