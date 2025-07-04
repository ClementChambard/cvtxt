#include "parser.hpp"
#include <cassert>
#include <ios>
#include <iostream>

std::string const &get_style_name(int val, CV const &cv) {
  int varid = -1;
  for (int i = 0; i < cv.variables.size(); i++) {
    if (cv.variables[i].val.kind != Value::STYLE)
      continue;
    if (cv.variables[i].val.val == val) {
      varid = i;
      break;
    }
  }
  assert(varid != -1);
  return cv.variables[varid].name;
}

void print_value(Value const &v, CV const &cv) {
  if (v.kind == Value::COLOR) {
    std::cout << '#' << std::hex << v.val << std::dec;
  } else if (v.kind == Value::INTEGER) {
    std::cout << v.val << '%';
  } else if (v.kind == Value::STYLE) {
    std::cout << '$' << get_style_name(v.val, cv);
  } else {
    std::cout << "'unk'";
  }
}

void print_layout(LayoutElem const &elt, CV const &cv, int offs = 0) {
  auto offs_str = std::string(offs, ' ');
  std::cout << offs_str << elt.kind;
  if (elt.name) {
    std::cout << ' ' << *elt.name;
  }
  if (!elt.props.empty()) {
    std::cout << " (";
    int i = 0;
    for (auto const &[k, v] : elt.props) {
      if (i != 0)
        std::cout << ", ";
      std::cout << k << " = ";
      print_value(v, cv);
      i++;
    }
    std::cout << ')';
  }
  if (!elt.children.empty()) {
    std::cout << " {\n";
    int i = 0;
    for (auto const &e : elt.children) {
      if (i != 0)
        std::cout << ",\n";
      print_layout(e, cv, offs + 2);
      i++;
    }
    std::cout << '\n' << offs_str << '}';
  }
}

void print_cv(CV const &cv) {
  for (int i = 0; i < cv.style.size(); i++) {
    std::cout << "%style " << get_style_name(i, cv) << " {";
    auto const &s = cv.style[i];
    if (s.values.size() > 0) {
      std::cout << '\n';
      for (auto const &[k, v] : s.values) {
        std::cout << "  " << k << " = ";
        print_value(v, cv);
        std::cout << ";\n";
      }
    }
    std::cout << "};\n\n";
  }

  for (auto const &l : cv.layout) {
    std::cout << "%layout ";
    if (l.name != "%root%")
      std::cout << l.name << ' ';
    print_layout(l.root, cv);
    std::cout << "\n";
  }
}

int main(int argc, char *argv[]) {
  Parser p;

  auto cv = p.read_cv_file(argv[1]);

  print_cv(cv);

  return 0;
}
