#include "parser.hpp"
#include "filedata.hpp"
#include <cassert>
#include <cstring>
#include <optional>

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
    p.consume_token();
    return Value(Value::COLOR, i32(val));
  } else if (p.tok.kind == Tok::DOLLAR) {
    p.consume_token();
    auto val = get_variable(out, p.tok.value);
    p.expect_and_consume(Tok::IDENT);
    return val;
  } else if (p.tok.kind == Tok::NUMBER) {
    // TODO: optional units
    auto val = parse_num_token(p.tok.value);
    auto kind = Value::NO_UNIT;
    p.consume_token();
    if (p.tok.kind == Tok::PERCENT) {
      p.consume_token();
      kind = Value::PC;
    } else if (p.tok.kind == Tok::IDENT && p.tok.value == "vw") {
      p.consume_token();
      kind = Value::VW;
    } else if (p.tok.kind == Tok::IDENT && p.tok.value == "vh") {
      p.consume_token();
      kind = Value::VH;
    }
    return Value(kind, f32(val));
  }
  // error
  return {};
}

void parse_vardecl(Parser &p, CV &out) {
  std::string_view var_name = p.tok.value;
  p.expect_and_consume(Tok::IDENT);
  p.expect_and_consume(Tok::EQUAL);
  auto value = parse_value(p, out);
  out.variables.push_back({std::string(var_name), value});
  p.expect_and_consume(Tok::SEMI);
}

void parse_style_rule(Parser &p, CV &out, Style &out_style) {
  assert(p.tok.kind == Tok::IDENT);
  auto name = p.tok.value;
  p.consume_token();
  p.expect_and_consume(Tok::EQUAL);
  auto value = parse_value(p, out);
  out_style.values[std::string(name)] = value;
  p.expect_and_consume(Tok::SEMI);
}

void parse_style(Parser &p, CV &out, std::optional<std::string_view> name) {
  p.expect_and_consume(Tok::LBRACE);
  Style s;
  while (p.tok.kind != Tok::RBRACE) {
    parse_style_rule(p, out, s);
  }
  p.expect_and_consume(Tok::RBRACE);
  out.style.push_back(s);
  if (name) {
    out.variables.push_back(
        {std::string(*name), {Value::STYLE, i32(out.style.size() - 1)}});
  }
  return;
}

void parse_prop_list(Parser &p, CV &out, LayoutElem &elt) {
  if (p.tok.kind == Tok::RPAREN)
    return;
  auto name = p.tok.value;
  p.expect_and_consume(Tok::IDENT);
  p.expect_and_consume(Tok::EQUAL);
  elt.props[std::string(name)] = parse_value(p, out);
  while (p.tok.kind == Tok::COMMA) {
    p.consume_token();
    if (p.tok.kind == Tok::RPAREN)
      break;
    auto name = p.tok.value;
    p.expect_and_consume(Tok::IDENT);
    p.expect_and_consume(Tok::EQUAL);
    elt.props[std::string(name)] = parse_value(p, out);
  }
}

LayoutElem parse_layout_elt(Parser &p, CV &out);

void parse_elt_list(Parser &p, CV &out, LayoutElem &elt) {
  if (p.tok.kind == Tok::RBRACE)
    return;
  elt.children.push_back(parse_layout_elt(p, out));
  while (p.tok.kind == Tok::COMMA) {
    p.consume_token();
    if (p.tok.kind == Tok::RBRACE)
      break;
    elt.children.push_back(parse_layout_elt(p, out));
  }
}

LayoutElem parse_layout_elt(Parser &p, CV &out) {
  LayoutElem elt;
  elt.kind = p.tok.value;
  p.expect_and_consume(Tok::IDENT);
  elt.name = std::nullopt;
  if (p.tok.kind == Tok::IDENT) {
    elt.name = p.tok.value;
    p.consume_token();
  }
  if (p.tok.kind == Tok::LPAREN) {
    p.consume_token();
    parse_prop_list(p, out, elt);
    p.expect_and_consume(Tok::RPAREN);
  }
  if (p.tok.kind == Tok::LBRACE) {
    p.consume_token();
    parse_elt_list(p, out, elt);
    p.expect_and_consume(Tok::RBRACE);
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
  p.expect_and_consume(Tok::PERCENT);
  auto tok = p.tok;
  p.expect_and_consume(Tok::IDENT);
  std::optional<std::string_view> name = std::nullopt;
  if (p.tok.kind == Tok::IDENT) {
    name = p.tok.value;
    p.consume_token();
  }
  p.expect_and_consume(Tok::EQUAL);
  if (tok.value == "style") {
    parse_style(p, out, name);
  } else if (tok.value == "layout") {
    parse_layout(p, out, name);
  } else {
    // ERROR
  }
  p.expect_and_consume(Tok::SEMI);
}

void Parser::unconsume_token(Token const &t) {
  auto next_tok = tok;
  l.enter_token(t);
  tok = l.lex();
  l.enter_token(next_tok);
}

Loc Parser::consume_token() {
  prev_tok_location = tok.loc;
  tok = l.lex();
  return prev_tok_location;
}

bool Parser::try_consume_token(Tok expected, Loc *loc_ptr) {
  if (tok.kind != expected)
    return false;
  auto loc = consume_token();
  if (loc_ptr)
    *loc_ptr = loc;
  return true;
}

bool Parser::expect_and_consume(Tok expected, std::string_view) {
  if (tok.kind == expected) {
    consume_token();
    return false;
  }
  // TODO: error msg
  assert(false);
}

void Parser::skip_until(std::span<Tok> until_toks) {
  while (true) {
    for (auto t : until_toks) {
      if (tok.kind == t) {
        // if !stop_before_match consume_token()
        return;
      }
    }
    switch (tok.kind) {
    case Tok::END:
      return;
    case Tok::LPAREN:
      consume_token();
      skip_until(Tok::RPAREN);
      consume_token();
      break;
    case Tok::LBRACE:
      consume_token();
      skip_until(Tok::RBRACE);
      consume_token();
      break;
    default:
      consume_token();
    }
  }
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
