#include "lexer.hpp"
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
  cur_pos = file_contents.data();
  end = cur_pos + file_contents.size() - 1;
}

void skip_whitespace(Lexer &l) {
  while (l.cur_pos != l.end && std::isspace(*l.cur_pos))
    l.cur_pos++;
}

Token finish_token(Lexer &l, char *end_pos, Tok kind) {
  auto size = end_pos - l.cur_pos;
  std::string_view data = std::string_view(l.cur_pos, size);
  l.cur_pos = end_pos;
  return {kind, data};
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

Token Lexer::lex() {
  skip_whitespace(*this);
  char *pos = cur_pos;
  char c = *pos++;
  switch (c) {
  case '\0':
    return finish_token(*this, pos, Tok::END);
  case '%':
    return finish_token(*this, pos, Tok::PERCENT);
  case '$':
    return finish_token(*this, pos, Tok::DOLLAR);
  case '=':
    return finish_token(*this, pos, Tok::EQUAL);
  case ';':
    return finish_token(*this, pos, Tok::SEMI);
  case ',':
    return finish_token(*this, pos, Tok::COMMA);
  case '(':
    return finish_token(*this, pos, Tok::LPAREN);
  case ')':
    return finish_token(*this, pos, Tok::RPAREN);
  case '{':
    return finish_token(*this, pos, Tok::LBRACE);
  case '}':
    return finish_token(*this, pos, Tok::RBRACE);
  case '#':
    return finish_color_token(*this, pos);
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
    return finish_num_token(*this, pos);
  default:
    if (is_identifier_start(c)) {
      return finish_ident_token(*this, pos);
    }
    // TODO: ERROR
  }
  return {};
}

#include <iostream>

void Token::print() {
  if (kind == Tok::END) {
    std::cout << "EOF";
    return;
  }
  if (kind == Tok::IDENT) {
    std::cout << "ident \"" << value << '"';
    return;
  }
  if (kind == Tok::IDENT) {
    std::cout << "num \"" << value << '"';
    return;
  }
  std::cout << '\'' << value << '\'';
}
