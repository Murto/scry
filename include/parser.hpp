#pragma once

#include "lexer.hpp"
#include "util.hpp"

#include <cstdio>

namespace initregex {

namespace ast {

/**
 * Structure representing a sequence of ASTs which must be satisfied in order
 */
template <typename... asts> struct sequence;

/**
 * Structure representing the none-or-more operation (*)
 */
template <typename ast> struct none_or_more;

/**
 * Structure representing the left-anchor (^)
 */
struct left_anchor;

/**
 * Structure representing the right-anchor ($)
 */
struct right_anchor;

/**
 * Structure representing the any character (.)
 */
struct any;

/**
 * Structure representing any single symbol
 */
template <char c> struct symbol;

} // namespace ast

template <typename ast, typename list> struct parser;

/**
 * Handle base case
 */
template <typename ast> struct parser<ast, list<>> { using type = ast; };

/**
 * Handle left-anchor circumflex circumflexes
 */
template <typename... ts>
struct parser<ast::sequence<>, list<token::circumflex, ts...>> {
  using type = typename parser<ast::sequence<>, list<ts...>>::type;
};

/**
 * Handle right-anchor dollar-signs
 */
template <typename... asts>
struct parser<ast::sequence<asts...>, list<token::dollar_sign>> {
  using type = typename parser<ast::sequence<asts...>, list<>>::type;
};

/**
 * Handle dots
 */
template <typename... asts, typename... ts>
struct parser<ast::sequence<asts...>, list<token::dot, ts...>> {
  using type =
      typename parser<ast::sequence<asts..., ast::any>, list<ts...>>::type;
};

/**
 * Handle asterisks with generic asts
 */
template <typename... asts, typename... ts>
struct parser<ast::sequence<asts...>, list<token::asterisk, ts...>> {
  using init_asts = typename init<ast::sequence<asts...>>::type;
  using last_ast = typename last<ast::sequence<asts...>>::type;
  using new_ast = typename ast::template none_or_more<last_ast>;
  using new_asts = typename append<init_asts, new_ast>::type;
  using type = typename parser<new_asts, list<ts...>>::type;
};

/**
 * Handle symbols
 */
template <typename... asts, char c, typename... ts>
struct parser<ast::sequence<asts...>, list<token::symbol<c>, ts...>> {
  using type = typename parser<ast::sequence<asts..., ast::symbol<c>>,
                               list<ts...>>::type;
};

template <typename list> struct parse_result {
  using type = typename parser<ast::sequence<>, list>::type;
};

} // namespace initregex
