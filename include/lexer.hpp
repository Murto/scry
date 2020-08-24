#pragma once

#include "util.hpp"

#include <utility>

namespace scry {

namespace token {

struct circumflex {};
struct dollar_sign {};
struct dot {};
struct asterisk {};
struct left_brace {};
struct right_brace {};
template <char c> struct symbol { constexpr static const char value = c; };

} // namespace token

/**
 * Structure used to lex a seqeuence of characters
 */
template <typename list, char... cs> struct lexer;

/**
 * Handle base case
 */
template <typename list> struct lexer<list> { using type = list; };

/**
 * Handle left-anchor circumflexes
 */
template <char... cs> struct lexer<list<>, '^', cs...> {
  using type = typename lexer<list<token::circumflex>, cs...>::type;
};

/**
 * Handle escaped circumflexes
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '^', cs...> {
  using type = typename lexer<list<ts..., token::symbol<'^'>>, cs...>::type;
};

/**
 * Disallow non-escaped circumflexes in the middle of a regex
 */
template <typename list, char... cs> struct lexer<list, '^', cs...>;

/**
 * Handle right-anchor dollar-signs
 */
template <typename... ts> struct lexer<list<ts...>, '$'> {
  using type = typename lexer<list<ts..., token::dollar_sign>>::type;
};

/**
 * Handle escaped dollar-signs
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '$', cs...> {
  using type = typename lexer<list<ts..., token::symbol<'$'>>, cs...>::type;
};

/**
 * Disallow non-escaped dollar-signs in the middle of a regex
 */
template <typename list, char... cs> struct lexer<list, '$', cs...>;

/**
 * Handle dot symbol
 */
template <typename... ts, char... cs> struct lexer<list<ts...>, '.', cs...> {
  using type = typename lexer<list<ts..., token::dot>, cs...>::type;
};

/**
 * Handle escaped dot symbols
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '.', cs...> {
  using type = typename lexer<list<ts..., token::symbol<'.'>>, cs...>::type;
};

/**
 * Handle asterisk duplicator
 */
template <typename... ts, char... cs> struct lexer<list<ts...>, '*', cs...> {
  using type = typename lexer<list<ts..., token::asterisk>, cs...>::type;
};

/**
 * Handle escaped asterisks
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '*', cs...> {
  using type = typename lexer<list<ts..., token::symbol<'*'>>, cs...>::type;
};

/**
 * Disallow asterisks as the initial character
 */
template <char... cs> struct lexer<list<>, '*', cs...>;

/**
 * Handle left braces
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '{', cs...> {
  using type = typename lexer<list<ts..., token::left_brace>, cs...>::type;
};

/**
 * Handle right braces
 */
template <typename... ts, char... cs>
struct lexer<list<ts...>, '\\', '}', cs...> {
  using type = typename lexer<list<ts..., token::right_brace>, cs...>::type;
};

/**
 * Handle non-special symbols
 */
template <typename... ts, char c, char... cs>
struct lexer<list<ts...>, c, cs...> {
  using type = typename lexer<list<ts..., token::symbol<c>>, cs...>::type;
};

template <typename regex, typename sequence> struct lex_result;

template <typename regex, std::size_t... n>
struct lex_result<regex, std::index_sequence<n...>> {
  using type = typename lexer<list<>, regex::string::get(n)...>::type;
};

} // namespace scry
