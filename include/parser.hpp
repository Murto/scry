#pragma once

#include "lexer.hpp"
#include "util.hpp"

#include <cstdio>

namespace scry {

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

template <typename ast, std::size_t n> struct interval;

template <typename ast, std::size_t n> struct lower_bound;

template <typename ast, std::size_t n, std::size_t m> struct bounded;

} // namespace ast

template <std::size_t n> struct number;

template <typename parts, typename tokens> struct interval_parser;

template <typename... ts>
struct interval_parser<list<>, list<token::left_brace, ts...>> {
  using interval_t = interval_parser<list<token::left_brace>, list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <char c, typename... ts>
struct interval_parser<list<token::left_brace>, list<token::symbol<c>, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<to_digit<c>::value>>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, char c, typename... ts>
struct interval_parser<list<token::left_brace, number<n>>,
                       list<token::symbol<c>, ts...>> {
  using interval_t = interval_parser<
      list<token::left_brace, number<n * 10 + to_digit<c>::value>>,
      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, typename... ts>
struct interval_parser<list<token::left_brace, number<n>>,
                       list<token::symbol<','>, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<n>, token::symbol<','>>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <char c, typename... ts>
struct interval_parser<list<token::left_brace, number<0>>,
                       list<token::symbol<c>, ts...>>;

template <std::size_t n, typename... ts>
struct interval_parser<list<token::left_brace, number<n>>,
                       list<token::right_brace, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<n>, token::right_brace>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, char c, typename... ts>
struct interval_parser<list<token::left_brace, number<n>, token::symbol<','>>,
                       list<token::symbol<c>, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<n>, token::symbol<','>,
                           number<to_digit<c>::value>>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, std::size_t m, char c, typename... ts>
struct interval_parser<
    list<token::left_brace, number<n>, token::symbol<','>, number<m>>,
    list<token::symbol<c>, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<n>, token::symbol<','>,
                           number<m * 10 + to_digit<c>::value>>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, std::size_t m, typename... ts>
struct interval_parser<
    list<token::left_brace, number<n>, token::symbol<','>, number<m>>,
    list<token::right_brace, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<n>, token::symbol<','>,
                           number<m>, token::right_brace>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, char c, typename... ts>
struct interval_parser<
    list<token::left_brace, number<n>, token::symbol<','>, number<0>>,
    list<token::symbol<c>, ts...>>;

template <std::size_t n, typename... ts>
struct interval_parser<list<token::left_brace, number<n>, token::symbol<','>>,
                       list<token::right_brace, ts...>> {
  using interval_t =
      interval_parser<list<token::left_brace, number<n>, token::symbol<','>,
                           token::right_brace>,
                      list<ts...>>;
  template <typename inner_ast>
  using type = typename interval_t::template type<inner_ast>;
  using tokens = typename interval_t::tokens;
};

template <std::size_t n, typename ts>
struct interval_parser<list<token::left_brace, number<n>, token::right_brace>,
                       ts> {
  template <typename inner_ast>
  using type = typename ast::template interval<inner_ast, n>;
  using tokens = ts;
};

template <std::size_t n, typename ts>
struct interval_parser<
    list<token::left_brace, number<n>, token::symbol<','>, token::right_brace>,
    ts> {
  template <typename inner_ast>
  using type = typename ast::template lower_bound<inner_ast, n>;
  using tokens = ts;
};

template <std::size_t n, std::size_t m, typename ts>
struct interval_parser<list<token::left_brace, number<n>, token::symbol<','>,
                            number<m>, token::right_brace>,
                       ts> {
  template <typename inner_ast>
  using type = typename ast::template bounded<inner_ast, n, m>;
  using tokens = ts;
};

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
 * Handle fixed intervals
 */
template <typename... asts, typename... ts>
struct parser<ast::sequence<asts...>, list<token::left_brace, ts...>> {
  using interval_t = interval_parser<list<>, list<token::left_brace, ts...>>;
  using init_asts = typename init<ast::sequence<asts...>>::type;
  using last_ast = typename last<ast::sequence<asts...>>::type;
  using new_ast = typename interval_t::template type<last_ast>;
  using new_asts = typename append<init_asts, new_ast>::type;
  using type = typename parser<new_asts, typename interval_t::tokens>::type;
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

} // namespace scry
