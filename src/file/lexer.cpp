#include "lexer.hpp"
#include "filedata.hpp"
#include <cctype>
#include <fstream>

std::string read_entire_file(char const *filename) {
  std::ifstream ifs(filename);
  ifs.seekg(0, std::ios::end);
  auto size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  std::string out(size, '\0');
  ifs.read(out.data(), out.size());
  ifs.close();
  return out;
}

void Lexer::open_file(char const *filename) {
  this->filename = filename;
  file_contents = read_entire_file(filename);
  file_contents += '\0';
  begin = file_contents.data();
  end = begin + file_contents.size() - 1;
  cur_pos = begin;
}

void skip_whitespace(Lexer &l) {
  while (l.cur_pos != l.end && std::isspace(*l.cur_pos))
    l.cur_pos++;
}

Token finish_token(Lexer &l, char *end_pos, Tok kind) {
  auto size = end_pos - l.cur_pos;
  auto loc = Loc(l.cur_pos - l.begin);
  std::string_view data = std::string_view(l.cur_pos, size);
  l.cur_pos = end_pos;
  return {kind, data, loc};
}

bool is_identifier_start(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
bool is_identifier_continue(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_';
}

Token finish_ident_token(Lexer &l, char *pos) {
  while (is_identifier_continue(*pos))
    pos++;
  return finish_token(l, pos, Tok::IDENT);
}

Token finish_num_token(Lexer &l, char *pos) {
  while (*pos >= '0' && *pos <= '9')
    pos++;
  return finish_token(l, pos, Tok::NUMBER);
}

Token finish_color_token(Lexer &l, char *pos) {
  while ((*pos >= '0' && *pos <= '9') || (*pos >= 'A' && *pos <= 'F') ||
         (*pos >= 'a' && *pos <= 'f'))
    pos++;
  return finish_token(l, pos, Tok::COLOR);
}

void Lexer::enter_token(Token const &t) { cached_tokens.push_back(t); }

void skip_until_newline(Lexer &l, char *pos) {
  while (*pos != '\n' && *pos != '\r')
    pos++;
  // maybe allow escaping newlines with '\'
  if (*pos == '\n' && *(pos + 1) == '\r')
    pos++;
  else if (*pos == '\r' && *(pos + 1) == '\n')
    pos++;
  l.cur_pos = pos + 1;
}

Token lex_no_cache(Lexer &l) {
  skip_whitespace(l);
  char *pos = l.cur_pos;
  char c = *pos++;
  switch (c) {
  case '\0':
    return finish_token(l, pos, Tok::END);
  case '%':
    if (*pos == '%') {
      // line comment
      skip_until_newline(l, pos + 1);
      return lex_no_cache(l);
    }
    return finish_token(l, pos, Tok::PERCENT);
  case '$':
    return finish_token(l, pos, Tok::DOLLAR);
  case '=':
    return finish_token(l, pos, Tok::EQUAL);
  case ';':
    return finish_token(l, pos, Tok::SEMI);
  case ',':
    return finish_token(l, pos, Tok::COMMA);
  case '(':
    return finish_token(l, pos, Tok::LPAREN);
  case ')':
    return finish_token(l, pos, Tok::RPAREN);
  case '{':
    return finish_token(l, pos, Tok::LBRACE);
  case '}':
    return finish_token(l, pos, Tok::RBRACE);
  case '#':
    return finish_color_token(l, pos);
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return finish_num_token(l, pos);
  default:
    if (is_identifier_start(c)) {
      return finish_ident_token(l, pos);
    }
    parsing_had_error = true;
    error_message += "unknown character 'c' in file\n"; // TODO: actually put character and put location
  }
  return {};
}

Token Lexer::lex() {
  if (cached_tokens.size() > 0) {
    Token out = cached_tokens.back();
    cached_tokens.pop_back();
    return out;
  }
  return lex_no_cache(*this);
}

Token const &Lexer::look_ahead(u32 n) {
  for (i32 i = 0; i < i32(n) - i32(cached_tokens.size()); i++) {
    cached_tokens.insert(cached_tokens.begin(), lex_no_cache(*this));
  }
  return cached_tokens[cached_tokens.size() - n];
}
