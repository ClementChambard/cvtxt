#ifndef FILEDATA_HPP
#define FILEDATA_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Value { // TODO:
  enum Kind {
    STYLE,
    INTEGER,
    COLOR,
  };
  int kind;
  int val;
};

struct LayoutElem {
  std::string kind;
  std::optional<std::string> name;
  std::unordered_map<std::string, Value> props;
  std::vector<LayoutElem> children;
};

struct Layout {
  std::string name;
  LayoutElem root;
};

struct Variable {
  std::string name;
  Value val;
};

struct Style {
  std::unordered_map<std::string, Value> values;
};

struct CV {
  // format (always A4 portrait for now)
  std::vector<Layout> layout;
  std::vector<Style> style;
  std::vector<Variable> variables;
};

#endif // !FILEDATA_HPP
