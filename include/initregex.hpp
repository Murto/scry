#pragma once

#include <optional>
#include <string>
#include <vector>

namespace initregex {

class regex {

  friend class match_visitor;

public:
  regex(char *pattern);

  regex(const std::string &pattern);

  regex(std::string &&pattern);

private:
  class simple_expr {

  public:
    simple_expr(char c, bool dupl);

  private:
    char c;
    bool dupl;
  };

  class regex_expr {

  public:
    regex_expr(bool l_anchor, const std::vector<simple_expr> &exprs,
               bool r_anchor);

  private:
    bool l_anchor;
    std::vector<simple_expr> exprs;
    bool r_anchor;
  };

  regex_expr expr;
};

class match_results {

public:
  match_results() = delete;

  bool ready() noexcept;

  bool empty() noexcept;

  std::string::size_type length();

  std::string str() { return *match; }

private:
  bool is_ready;
  std::optional<std::string> match;
};

class match_visitor {

public:
  template <typename type> match_results visit(const type &r) {
    throw std::runtime_error{"Unsupported type"};
  }

  match_results visit(const regex::simple_expr &r) {
    throw std::runtime_error{"Unimplemented"};
  }

  match_results visit(const regex::regex_expr &r) {
    throw std::runtime_error{"Unimplemented"};
  }
};

template <typename it_type>
bool regex_match(it_type begin, it_type end, regex r) {
  return false;
}

template <typename it_type>
bool regex_match(it_type begin, it_type end, match_results m, regex r) {
  return false;
}

bool regex_match(const std::string &s, regex r);

bool regex_match(const std::string &s, match_results m, regex r);

} // namespace initregex
