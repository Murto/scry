#pragma once

#include "definitions.hpp"
#include "util.hpp"

namespace scry {

namespace ast {

template <typename...> struct sequence;
template <char c> struct symbol;
struct any;
template <typename nested> struct zero_or_more;
struct left_anchor;
struct right_anchor;
template <std::size_t n, typename nested> struct exactly;
template <std::size_t n, typename nested> struct at_least;
template <std::size_t n, std::size_t m, typename nested> struct between;
template <typename...> struct any_of;
template <typename...> struct none_of;

} // namespace ast

namespace {

/**
 * TEMPLATES FOR REGEX PARSING
 *
 * The templates below are used only for parsing regular expressions.
 * Optimisation will be done at some point in the future via another set of
 * templates based on matching types which lend themselves to being replaced
 * with more optimally executed types.
 */

/**
 * Helper struct to convert char values into types
 */
template <char c> struct token;

/**
 * Helper struct to convert number values into types
 */
template <std::size_t n> struct number;

/**
 * Structure used for parsing brace expressions
 */
template <typename parts, typename tokens> struct parse_brcex;

/**
 * Structure used for parsing bracket expressions
 */
template <typename parts, typename tokens> struct parse_brkex;

/**
 * Handle open-brace digraph ('\{')
 */
template <typename... tokens>
struct parse_brcex<list<>, list<token<'\\'>, token<'{'>, tokens...>> {
  using brcex = parse_brcex<list<token<'{'>>, list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

/**
 * Handle lower bound number
 */
template <char c, typename... tokens>
struct parse_brcex<list<token<'{'>>, list<token<c>, tokens...>> {
  using brcex = parse_brcex<list<token<'{'>, number<to_digit<c>::value>>,
                            list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

template <std::size_t n, char c, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>>, list<token<c>, tokens...>> {
  using brcex =
      parse_brcex<list<token<'{'>, number<n * 10 + to_digit<c>::value>>,
                  list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

template <char c, typename... tokens>
struct parse_brcex<list<token<'{'>, number<0>>, list<token<c>, tokens...>>;

/**
 * Handle comma character (',')
 */
template <std::size_t n, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>>, list<token<','>, tokens...>> {
  using brcex =
      parse_brcex<list<token<'{'>, number<n>, token<','>>, list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

/**
 * Handle upper bound number
 */
template <std::size_t n, char c, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<','>>,
                   list<token<c>, tokens...>> {
  using brcex = parse_brcex<
      list<token<'{'>, number<n>, token<','>, number<to_digit<c>::value>>,
      list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};
template <std::size_t n, size_t m, char c, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<','>, number<m>>,
                   list<token<c>, tokens...>> {
  using brcex = parse_brcex<list<token<'{'>, number<n>, token<','>,
                                 number<m * 10 + to_digit<c>::value>>,
                            list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};
template <std::size_t n, char c, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<','>, number<0>>,
                   list<token<c>, tokens...>>;

/**
 * Handle cases for closing digraph ('\}')
 */
template <std::size_t n, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>>,
                   list<token<'\\'>, token<'}'>, tokens...>> {
  using brcex =
      parse_brcex<list<token<'{'>, number<n>, token<'}'>>, list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

template <std::size_t n, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<','>>,
                   list<token<'\\'>, token<'}'>, tokens...>> {
  using brcex = parse_brcex<list<token<'{'>, number<n>, token<','>, token<'}'>>,
                            list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

template <std::size_t n, std::size_t m, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<','>, number<m>>,
                   list<token<'\\'>, token<'}'>, tokens...>> {
  using brcex = parse_brcex<
      list<token<'{'>, number<n>, token<','>, number<m>, token<'}'>>,
      list<tokens...>>;
  template <typename nested> using type = typename brcex::template type<nested>;
  using unused = typename brcex::unused;
};

/**
 * Handle all three cases for complete brace expressions
 *
 * Case 1: repetition '{n}'
 * Case 2: lower-bounded repetition '{n,}'
 * Case 1: lower- and upper-bounded repetition '{n,m}'
 */
template <std::size_t n, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<'}'>>, list<tokens...>> {
  template <typename nested> using type = ast::exactly<n, nested>;
  using unused = list<tokens...>;
};

template <std::size_t n, typename... tokens>
struct parse_brcex<list<token<'{'>, number<n>, token<','>, token<'}'>>,
                   list<tokens...>> {
  template <typename nested> using type = ast::at_least<n, nested>;
  using unused = list<tokens...>;
};

template <std::size_t n, std::size_t m, typename... tokens>
struct parse_brcex<
    list<token<'{'>, number<n>, token<','>, number<m>, token<'}'>>,
    list<tokens...>> {
  template <typename nested> using type = ast::between<n, m, nested>;
  using unused = list<tokens...>;
};

/**
 * Handle initial open-bracket
 */
template <typename... tokens>
struct parse_brkex<list<>, list<token<'['>, tokens...>> {
  using brkex = parse_brkex<list<ast::symbol<'['>>, list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle generic symbols
 */
template <typename... parts, char c, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, parts...>,
                   list<token<c>, tokens...>> {
  using brkex = parse_brkex<list<ast::symbol<'['>, parts..., ast::symbol<c>>,
                            list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle close-bracket character (']')
 */
template <typename... parts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, parts...>,
                   list<token<']'>, tokens...>> {
  using type = ast::any_of<parts...>;
  using unused = list<tokens...>;
};

/**
 * Handle immediate close-bracket character
 */
template <typename... tokens>
struct parse_brkex<list<ast::symbol<'['>>, list<token<']'>, tokens...>> {
  using brkex =
      parse_brkex<list<ast::symbol<'['>, ast::symbol<']'>>, list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Structure used for parsing regular expressions
 */
template <typename ast, typename tokens> struct parse_regex;

/**
 * Handle end of input
 */
template <typename ast> struct parse_regex<ast, list<>> { using type = ast; };

/**
 * Handle generic characters
 */
template <typename... asts, char c, typename... tokens>
struct parse_regex<ast::sequence<asts...>, list<token<c>, tokens...>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::symbol<c>>,
                                    list<tokens...>>::type;
};

/**
 * Handle dot character ('.')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>, list<token<'.'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::any>,
                                    list<tokens...>>::type;
};

/**
 * Handle escaped dot character ('\.')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>,
                   list<token<'\\'>, token<'.'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::symbol<'.'>>,
                                    list<tokens...>>::type;
};

/**
 * Handle escaped escape character ('\\')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>,
                   list<token<'\\'>, token<'\\'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::symbol<'\\'>>,
                                    list<tokens...>>::type;
};

/**
 * Disallow invalid escape character ('\')
 */
template <typename ast, typename... tokens>
struct parse_regex<ast, list<token<'\\'>, tokens...>>;

/**
 * Handle asterisk character ('*')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>, list<token<'*'>, tokens...>> {
  using init_asts = typename init<ast::sequence<asts...>>::type;
  using last_ast = typename last<ast::sequence<asts...>>::type;
  using new_ast = typename ast::zero_or_more<last_ast>;
  using new_asts = typename append<init_asts, new_ast>::type;
  using type = typename parse_regex<new_asts, list<tokens...>>::type;
};

/**
 * Handle circumflex character ('^')
 */
template <typename... tokens>
struct parse_regex<ast::sequence<>, list<token<'^'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<ast::left_anchor>,
                                    list<tokens...>>::type;
};

/**
 * Handle escaped circumflex character ('\^')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>,
                   list<token<'\\'>, token<'^'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::symbol<'^'>>,
                                    list<tokens...>>::type;
};

/**
 * Handle dollar-sign character ('$')
 */
template <typename... asts>
struct parse_regex<ast::sequence<asts...>, list<token<'$'>>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::right_anchor>,
                                    list<>>::type;
};

/**
 * Handle escaped dollar-sign character ('$')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>,
                   list<token<'\\'>, token<'$'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<asts..., ast::symbol<'$'>>,
                                    list<tokens...>>::type;
};

/**
 * Handle escaped open-brace digraph ('\{')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>,
                   list<token<'\\'>, token<'{'>, tokens...>> {
  using brcex = parse_brcex<list<>, list<token<'\\'>, token<'{'>, tokens...>>;
  using init_asts = typename init<ast::sequence<asts...>>::type;
  using last_ast = typename last<ast::sequence<asts...>>::type;
  using new_ast = typename brcex::template type<last_ast>;
  using new_asts = typename append<init_asts, new_ast>::type;
  using type = typename parse_regex<new_asts, typename brcex::unused>::type;
};

/**
 * Handle open-bracket character ('[')
 */
template <typename... asts, typename... tokens>
struct parse_regex<ast::sequence<asts...>, list<token<'['>, tokens...>> {
  using brkex = parse_brkex<list<>, list<token<'['>, tokens...>>;
  using type =
      typename parse_regex<ast::sequence<asts..., typename brkex::type>,
                           typename brkex::unused>::type;
};

} // anonymous namespace

/**
 * Top level structure for parsing regular expressions
 */
template <typename regex, typename sequence> struct parse_result;

template <typename regex, std::size_t... n>
struct parse_result<regex, std::index_sequence<n...>> {
  using type =
      typename parse_regex<ast::sequence<>,
                           list<token<regex::string::get(n)>...>>::type;
};

} // namespace scry
