#ifndef LEXER_HPP
#define LEXER_HPP

#include "../defines.hpp"
#include <string>
#include <string_view>
#include <vector>

using Loc = i32;

enum class Tok {
  IDENT,
  NUMBER,
  COLOR,
  UNIT,
  PERCENT,
  DOLLAR,
  EQUAL,
  SEMI,
  COMMA,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  END
};

struct Token {
  Tok kind;
  std::string_view value;
  Loc loc;
};

struct Lexer {
  std::string filename;
  std::string file_contents;
  char *begin;
  char *cur_pos;
  char *end;
  std::vector<Token> cached_tokens;
  void open_file(char const *filename);
  void enter_token(Token const &t);
  Token const &look_ahead(u32 n);
  Token lex();
};

#endif // !LEXER_HPP
