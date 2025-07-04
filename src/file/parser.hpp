#ifndef PARSER_HPP
#define PARSER_HPP

#include "filedata.hpp"
#include "lexer.hpp"
#include <span>

struct Parser {
  Lexer l;
  Token tok;
  Loc prev_tok_location;
  void unconsume_token(Token const &t);
  Loc consume_token();
  bool try_consume_token(Tok expected, Loc *loc_ptr = nullptr);
  bool expect_and_consume(Tok expected,
                          std::string_view message = "expected %s");
  Token const &next_token() { return l.look_ahead(1); }
  void skip_until(std::span<Tok> end_toks);
  void skip_until(Tok t1) { skip_until(std::span<Tok>(&t1, 1)); }
  void skip_until(Tok t1, Tok t2) {
    Tok ts[] = {t1, t2};
    skip_until(ts);
  }
  void skip_until(Tok t1, Tok t2, Tok t3) {
    Tok ts[] = {t1, t2, t3};
    skip_until(ts);
  }
  CV read_cv_file(char const *filename);
};

#endif // !PARSER_HPP
