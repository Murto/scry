#include "initregex.hpp"

#include <stdexcept>

namespace initregex {

template <typename it_type>
static std::pair<simple_expr, it_type> parse_simple_expr(it_type begin,
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
  if (c == '.') {
    return {{any_char_type{}, dupl}, it};
  } else {
    return {{c, dupl}, it};
  }
}

template <typename it_type>
static regex_expr parse_regex_expr(it_type begin, it_type end) {
  if (begin == end) {
    throw std::runtime_error{"Unexpected end of input"};
  }
  auto l_anchor = false;
  if (*begin == '^') {
    l_anchor = true;
    ++begin;
  }
  std::vector<simple_expr> exprs;
  if (begin != end) {
    while (begin != end && *begin != '$') {
      auto [expr, it] = parse_simple_expr(begin, end);
      exprs.push_back(expr);
      begin = it;
    }
  } else if (!l_anchor) {
    throw std::runtime_error{"Expected simple expression"};
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

nfa<char> expr_to_nfa(const regex_expr &expr) {
  nfa_visitor<char> visitor;
  visitor.visit(expr);
  return visitor.acceptor;
}

regex::regex(char *pattern) : regex(std::move(std::string(pattern))) {}

regex::regex(const std::string &pattern)
    : acceptor{expr_to_nfa(parse_regex_expr(pattern.begin(), pattern.end()))} {}

regex::regex(std::string &&pattern)
    : acceptor{expr_to_nfa(parse_regex_expr(pattern.begin(), pattern.end()))} {}

regex_expr::regex_expr(bool l_anchor, const std::vector<simple_expr> &exprs,
                       bool r_anchor)
    : l_anchor{l_anchor}, exprs{exprs}, r_anchor{r_anchor} {}

simple_expr::simple_expr(value_type c, bool dupl) : c{c}, dupl{dupl} {}

bool match_results::ready() noexcept { return is_ready; }

bool match_results::empty() noexcept { return !match.has_value(); }

std::string::size_type match_results::length() { return match->size(); }

bool regex_match(const std::string &s, regex r) {
  return regex_match(s.begin(), s.end(), r);
}

bool regex_match(const std::string &s, match_results m, regex r) {
  return regex_match(s.begin(), s.end(), m, r);
}

} // namespace initregex
