#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <string_view>

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

  void print();
};

struct Lexer {
  std::string filename;
  std::string file_contents;
  char *cur_pos;
  char *end;
  void open_file(char const *filename);
  Token lex();
};

#endif // !LEXER_HPP
