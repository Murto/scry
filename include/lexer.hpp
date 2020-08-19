#pragma once

#include "util.hpp"

#include <utility>

namespace initregex {

struct circumflex {};
struct dollar_sign {};
struct dot {};
struct asterisk {};

template <char c> struct symbol { constexpr static const char value = c; };

template <typename list, char... cs> struct lexer;

/**
 * Handle base case
 */
template <typename list> struct lexer<list> { using tokens = list; };

/**
 * Handle left-anchor circumflexes
 */
template <char... cs> struct lexer<list<>, '^', cs...> {
  using tokens = typename lexer<list<circumflex>, cs...>::tokens;
};

/**
 * Handle escaped circumflexes
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '^', cs...> {
  using tokens = typename lexer<list<ts..., symbol<'^'>>, cs...>::tokens;
};

/**
 * Disallow non-escaped circumflexes in the middle of a regex
 */
template <typename list, char... cs> struct lexer<list, '^', cs...>;

/**
 * Handle right-anchor dollar-signs
 */
template <typename... ts> struct lexer<list<ts...>, '$'> {
  using tokens = typename lexer<list<ts..., dollar_sign>>::tokens;
};

/**
 * Handle escaped dollar-signs
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '$', cs...> {
  using tokens = typename lexer<list<ts..., symbol<'$'>>, cs...>::tokens;
};

/**
 * Disallow non-escaped dollar-signs in the middle of a regex
 */
template <typename list, char... cs> struct lexer<list, '$', cs...>;

/**
 * Handle dot symbol
 */
template <typename... ts, char... cs> struct lexer<list<ts...>, '.', cs...> {
  using tokens = typename lexer<list<ts..., dot>, cs...>::tokens;
};

/**
 * Handle escaped dot symbols
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '.', cs...> {
  using tokens = typename lexer<list<ts..., symbol<'.'>>, cs...>::tokens;
};

/**
 * Handle asterisk duplicator
 */
template <typename... ts, char... cs> struct lexer<list<ts...>, '*', cs...> {
  using tokens = typename lexer<list<ts..., asterisk>, cs...>::tokens;
};

/**
 * Handle escapted asterisks
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '*', cs...> {
  using tokens = typename lexer<list<ts..., symbol<'*'>>, cs...>::tokens;
};

/**
 * disallow asterisks as the initial character
 */
template <char... cs> struct lexer<list<>, '*', cs...>;

/**
 * Handle non-special symbols
 */
template <typename... ts, char c, char... cs>
struct lexer<list<ts...>, c, cs...> {
  using tokens = typename lexer<list<ts..., symbol<c>>, cs...>::tokens;
};

template <typename regex, typename sequence> struct lex_result;

template <typename regex, std::size_t... n>
struct lex_result<regex, std::index_sequence<n...>> {
  using tokens = typename lexer<list<>, regex::string::get(n)...>::tokens;
};

} // namespace initregex
