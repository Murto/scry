#include "initregex.hpp"

#include <stdexcept>

namespace initregex {

template <typename it_type>
static std::pair<regex::simple_expr, it_type> parse_simple_expr(it_type begin,
                                                                it_type end) {
  it_type it = begin;
  if (*it == '*' || *it == '^' || *it == '$') {
    throw std::runtime_error{"Unexpected token"};
  }
  auto c = *it;
  ++it;
  auto dupl = false;
  if (*it == '*') {
    dupl = true;
    ++it;
  }
  return {{c, dupl}, it};
}

template <typename it_type>
static regex::regex_expr parse_regex_expr(it_type begin, it_type end) {
  if (begin == end) {
    throw std::runtime_error{"Unexpected end of input"};
  }
  auto l_anchor = false;
  if (*begin == '^') {
    l_anchor = true;
    ++begin;
  }
  std::vector<regex::simple_expr> exprs;
  while (begin != end && *begin != '$') {
    auto [expr, it] = parse_simple_expr(begin, end);
    exprs.push_back(expr);
    begin = it;
  }
  auto r_anchor = false;
  if (begin != end) {
    r_anchor = true;
    ++begin;
  }
  if (begin != end) {
    throw std::runtime_error{"Expected end of input"};
  }
  return {l_anchor, exprs, r_anchor};
}

regex::regex(char *pattern) : regex(std::move(std::string(pattern))) {}

regex::regex(const std::string &pattern)
    : expr{parse_regex_expr(std::begin(pattern), std::end(pattern))} {}

regex::regex(std::string &&pattern)
    : expr{parse_regex_expr(std::begin(pattern), std::end(pattern))} {}

regex::regex_expr::regex_expr(bool l_anchor,
                              const std::vector<simple_expr> &exprs,
                              bool r_anchor)
    : l_anchor{l_anchor}, exprs{exprs}, r_anchor{r_anchor} {}

regex::simple_expr::simple_expr(char c, bool dupl) : c{c}, dupl{dupl} {}

} // namespace initregex
