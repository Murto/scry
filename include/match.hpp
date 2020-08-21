#pragma once

#include "ct_string.hpp"
#include "generation.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "regex.hpp"

#include <cassert>
#include <string>
#include <utility>

namespace scry {

namespace {

/**
 * Helper to get begin iterator of const char pointer
 */
const char *begin(const char *str) { return str; }

/**
 * Helper to get the end iterator of a const char pointer
 */
const char *end(const char *str) {
  while (*str != '\0')
    ++str;
  return str;
}

} // namespace

template <typename regex, typename it_type>
bool regex_match(it_type begin, it_type end) noexcept {
  using tokens =
      typename lex_result<regex,
                          std::make_index_sequence<regex::string::size>>::type;
  using tree = typename parse_result<tokens>::type;
  using exec = typename generation_result<tree>::type;
  return exec::execute(begin, end);
}

template <typename regex> bool regex_match(const char *str) noexcept {
  return regex_match<regex>(begin(str), end(str));
}

template <typename regex> bool regex_match(std::string str) noexcept {
  return regex_match<regex>(str.begin(), str.end());
}

} // namespace scry
