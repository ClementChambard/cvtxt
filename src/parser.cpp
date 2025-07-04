#include "parser.hpp"
#include "filedata.hpp"
#include <cassert>
#include <cstring>
#include <optional>

Token &Parser::next_token() {
  tok = l.lex();
  return tok;
}

void expect_token(Parser &p, Tok kind) {
  assert(p.tok.kind == kind);
  p.next_token();
}

int parse_color_token(std::string_view value) {
  assert(value[0] == '#');
  value = value.substr(1);
  char buf[10] = {'#', '0', '0', '0', '0', '0', '0', 'f', 'f', '\0'};
  if (value.size() == 1) {
    buf[1] = buf[2] = buf[3] = buf[4] = buf[5] = buf[6] = value[0];
    return parse_color_token(buf);
  }
  if (value.size() == 2) {
    buf[1] = buf[3] = buf[5] = value[0];
    buf[2] = buf[4] = buf[6] = value[1];
    return parse_color_token(buf);
  }
  if (value.size() == 3) {
    buf[1] = buf[2] = value[0];
    buf[3] = buf[4] = value[1];
    buf[5] = buf[6] = value[2];
    return parse_color_token(buf);
  }
  if (value.size() == 4) {
    buf[1] = buf[2] = value[0];
    buf[3] = buf[4] = value[1];
    buf[5] = buf[6] = value[2];
    buf[7] = buf[8] = value[3];
    return parse_color_token(buf);
  }
  if (value.size() == 6) {
    std::memcpy(&buf[1], value.data(), 6);
    return parse_color_token(buf);
  }
  if (value.size() == 8) {
    int res = 0;
    for (auto i = 0; i < 8; i++) {
      auto c = value[i];
      int hex_digit = 0;
      if (c >= '0' && c <= '9')
        hex_digit = c - '0';
      if (c >= 'A' && c <= 'F')
        hex_digit = c - 'A' + 0xA;
      if (c >= 'a' && c <= 'f')
        hex_digit = c - 'a' + 0xa;
      res = res * 0x10 + hex_digit;
    }
    return res;
  }
  // ERROR
  return 0;
}

int parse_num_token(std::string_view value) {
  int res = 0;
  for (auto c : value) {
    res = res * 10 + (c - '0');
  }
  return res;
}

Value get_variable(CV &out, std::string_view name) {
  for (auto const &v : out.variables) {
    if (v.name == name)
      return v.val;
  }
  // ERROR
  return {};
}

Value parse_value(Parser &p, CV &out) {
  if (p.tok.kind == Tok::COLOR) {
    auto val = parse_color_token(p.tok.value);
    p.next_token();
    return {Value::COLOR, val};
  } else if (p.tok.kind == Tok::DOLLAR) {
    p.next_token();
    assert(p.tok.kind == Tok::IDENT);
    auto val = get_variable(out, p.tok.value);
    p.next_token();
    return val;
  } else if (p.tok.kind == Tok::NUMBER) {
    // TODO: optional units
    auto val = parse_num_token(p.tok.value);
    p.next_token();
    expect_token(p, Tok::PERCENT);
    return {Value::INTEGER, val};
  }
  // error
  return {};
}

void parse_vardecl(Parser &p, CV &out) {
  std::string_view var_name = p.tok.value;
  p.next_token();
  expect_token(p, Tok::EQUAL);
  auto value = parse_value(p, out);
  out.variables.push_back({std::string(var_name), value});
  expect_token(p, Tok::SEMI);
}

void parse_style_rule(Parser &p, CV &out, Style &out_style) {
  assert(p.tok.kind == Tok::IDENT);
  auto name = p.tok.value;
  p.next_token();
  expect_token(p, Tok::EQUAL);
  auto value = parse_value(p, out);
  out_style.values[std::string(name)] = value;
  expect_token(p, Tok::SEMI);
}

void parse_style(Parser &p, CV &out, std::optional<std::string_view> name) {
  expect_token(p, Tok::LBRACE);
  Style s;
  while (p.tok.kind != Tok::RBRACE) {
    parse_style_rule(p, out, s);
  }
  expect_token(p, Tok::RBRACE);
  out.style.push_back(s);
  if (name) {
    out.variables.push_back(
        {std::string(*name), {Value::STYLE, int(out.style.size() - 1)}});
  }
  return;
}

void parse_prop_list(Parser &p, CV &out, LayoutElem &elt) {
  if (p.tok.kind == Tok::RPAREN)
    return;
  assert(p.tok.kind == Tok::IDENT);
  auto name = p.tok.value;
  p.next_token();
  expect_token(p, Tok::EQUAL);
  elt.props[std::string(name)] = parse_value(p, out);
  while (p.tok.kind == Tok::COMMA) {
    p.next_token();
    if (p.tok.kind == Tok::RPAREN)
      break;
    assert(p.tok.kind == Tok::IDENT);
    auto name = p.tok.value;
    p.next_token();
    expect_token(p, Tok::EQUAL);
    elt.props[std::string(name)] = parse_value(p, out);
  }
}

LayoutElem parse_layout_elt(Parser &p, CV &out);

void parse_elt_list(Parser &p, CV &out, LayoutElem &elt) {
  if (p.tok.kind == Tok::RBRACE)
    return;
  elt.children.push_back(parse_layout_elt(p, out));
  while (p.tok.kind == Tok::COMMA) {
    if (p.tok.kind == Tok::RBRACE)
      break;
    p.next_token();
    elt.children.push_back(parse_layout_elt(p, out));
  }
}

LayoutElem parse_layout_elt(Parser &p, CV &out) {
  LayoutElem elt;
  assert(p.tok.kind == Tok::IDENT);
  elt.kind = p.tok.value;
  elt.name = std::nullopt;
  p.next_token();
  if (p.tok.kind == Tok::IDENT) {
    elt.name = p.tok.value;
    p.next_token();
  }
  if (p.tok.kind == Tok::LPAREN) {
    p.next_token();
    parse_prop_list(p, out, elt);
    expect_token(p, Tok::RPAREN);
  }
  if (p.tok.kind == Tok::LBRACE) {
    p.next_token();
    parse_elt_list(p, out, elt);
    expect_token(p, Tok::RBRACE);
  }
  return elt;
}

void parse_layout(Parser &p, CV &out, std::optional<std::string_view> name) {
  auto root_elt = parse_layout_elt(p, out);
  std::string layout_name = "%root%";
  if (name)
    layout_name = *name;
  out.layout.push_back({layout_name, root_elt});
}

void parse_pcdecl(Parser &p, CV &out) {
  auto tok = p.next_token();
  assert(tok.kind == Tok::IDENT);
  p.next_token();
  std::optional<std::string_view> name = std::nullopt;
  if (p.tok.kind == Tok::IDENT) {
    name = p.tok.value;
    p.next_token();
  }
  expect_token(p, Tok::EQUAL);
  if (tok.value == "format") {
    // TODO:
    while (p.tok.kind != Tok::SEMI)
      p.next_token();
    expect_token(p, Tok::SEMI);
    return;
  } else if (tok.value == "style") {
    parse_style(p, out, name);
  } else if (tok.value == "layout") {
    parse_layout(p, out, name);
  } else {
    // ERROR
  }
  expect_token(p, Tok::SEMI);
}

CV Parser::read_cv_file(char const *filename) {
  l.open_file(filename);

  tok = l.lex();

  CV out;

  while (true) {
    if (tok.kind == Tok::PERCENT) {
      parse_pcdecl(*this, out);
    } else if (tok.kind == Tok::IDENT) {
      parse_vardecl(*this, out);
    } else if (tok.kind == Tok::END) {
      break;
    } else {
      // ERROR!
    }
  }
  return out;
}
