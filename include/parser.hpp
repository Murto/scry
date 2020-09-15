#pragma once

#include "definitions.hpp"
#include "util.hpp"

namespace scry {

namespace ast {

template <typename...> struct sequence;
template <char c> struct symbol { constexpr static char value = c; };
struct any;
template <typename nested> struct zero_or_more;
struct left_anchor;
struct right_anchor;
template <std::size_t n, typename nested> struct exactly;
template <std::size_t n, typename nested> struct at_least;
template <std::size_t n, typename nested> struct at_most;
template <typename...> struct any_of;
template <typename...> struct none_of;
template <char lower, char upper> struct range {
  static_assert(lower <= upper, "Invalid range expression");
};

namespace cc {

using upper = range<'A', 'Z'>;
using lower = range<'a', 'z'>;
using alpha = any_of<upper, lower>;
using digit = range<'0', '9'>;
using xdigit = any_of<digit, range<'a', 'f'>, range<'A', 'F'>>;
using alnum = any_of<upper, lower, digit>;
using punct = any_of<range<0x21, 0x2F>, range<0x3A, 0x40>, range<0x5B, 0x60>,
                     range<0x7B, 0x7E>>;
using blank = any_of<symbol<' '>, symbol<'\t'>>;
using space = any_of<symbol<' '>, symbol<'\t'>, symbol<'\n'>, symbol<'\r'>,
                     symbol<'\f'>, symbol<'\v'>>;
using cntrl = range<0x00, 0x1F>;
using graph = any_of<alnum, punct>;
using print = any_of<graph, symbol<' '>>;
using word = any_of<alnum, symbol<'_'>>;

} // namespace cc

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
 * Helper struct for transforming character sequences into their respective
 * character classes
 */
template <typename...> struct claex_map;

template <>
struct claex_map<token<'u'>, token<'p'>, token<'p'>, token<'e'>, token<'r'>> {
  using type = ast::cc::upper;
};

template <>
struct claex_map<token<'l'>, token<'o'>, token<'w'>, token<'e'>, token<'r'>> {
  using type = ast::cc::lower;
};

template <>
struct claex_map<token<'a'>, token<'l'>, token<'p'>, token<'h'>, token<'a'>> {
  using type = ast::cc::alpha;
};

template <>
struct claex_map<token<'d'>, token<'i'>, token<'g'>, token<'i'>, token<'t'>> {
  using type = ast::cc::digit;
};

template <>
struct claex_map<token<'x'>, token<'d'>, token<'i'>, token<'g'>, token<'i'>,
                 token<'t'>> {
  using type = ast::cc::xdigit;
};

template <>
struct claex_map<token<'a'>, token<'l'>, token<'n'>, token<'u'>, token<'m'>> {
  using type = ast::cc::alnum;
};

template <>
struct claex_map<token<'p'>, token<'u'>, token<'n'>, token<'c'>, token<'t'>> {
  using type = ast::cc::punct;
};

template <>
struct claex_map<token<'b'>, token<'l'>, token<'a'>, token<'n'>, token<'k'>> {
  using type = ast::cc::blank;
};

template <>
struct claex_map<token<'s'>, token<'p'>, token<'a'>, token<'c'>, token<'e'>> {
  using type = ast::cc::space;
};

template <>
struct claex_map<token<'c'>, token<'n'>, token<'t'>, token<'r'>, token<'l'>> {
  using type = ast::cc::cntrl;
};

template <>
struct claex_map<token<'g'>, token<'r'>, token<'a'>, token<'p'>, token<'h'>> {
  using type = ast::cc::graph;
};

template <>
struct claex_map<token<'p'>, token<'r'>, token<'i'>, token<'n'>, token<'t'>> {
  using type = ast::cc::print;
};

template <> struct claex_map<token<'w'>, token<'o'>, token<'r'>, token<'d'>> {
  using type = ast::cc::word;
};

/**
 * Structure used for parsing character class expressions
 */
template <typename parts, typename tokens> struct parse_ccex;

/**
 * Structure used for parsing brace expressions
 */
template <typename parts, typename tokens> struct parse_brcex;

/**
 * Structure used for parsing bracket expressions
 */
template <typename parts, typename tokens> struct parse_brkex;

/**
 * Handle open-character-class digraph ('[:')
 */
template <typename... tokens>
struct parse_ccex<list<>, list<token<'['>, token<':'>, tokens...>> {
  using claex = parse_ccex<list<token<'['>, token<':'>>, list<tokens...>>;
  using type = typename claex::type;
  using unused = typename claex::unused;
};

/**
 * Handle characters
 */
template <typename... parts, char c, typename... tokens>
struct parse_ccex<list<token<'['>, token<':'>, parts...>,
                  list<token<c>, tokens...>> {
  using claex = parse_ccex<list<token<'['>, token<':'>, parts..., token<c>>,
                           list<tokens...>>;
  using type = typename claex::type;
  using unused = typename claex::unused;
};

/**
 * Handle close-character-class digraph (':]')
 */
template <typename... parts, typename... tokens>
struct parse_ccex<list<token<'['>, token<':'>, parts...>,
                  list<token<':'>, token<']'>, tokens...>> {
  using type = typename claex_map<parts...>::type;
  using unused = list<tokens...>;
};

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
  template <typename nested>
  using type =
      ast::sequence<ast::exactly<n, nested>, ast::at_most<m - n, nested>>;
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
 * Handle generic tokens
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
 * Handle escaped tokens
 */
template <typename... parts, char c, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, parts...>,
                   list<token<'\\'>, token<c>, tokens...>> {
  using brkex = parse_brkex<list<ast::symbol<'['>, parts..., ast::symbol<c>>,
                            list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
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
 * Handle range expressions
 */
template <typename... asts, char c, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'-'>, token<c>, tokens...>> {
  using init_asts = typename init<list<ast::symbol<'['>, asts...>>::type;
  using last_ast = typename last<list<asts...>>::type;
  using new_ast = typename ast::range<last_ast::value, c>;
  using new_asts = typename append<init_asts, new_ast>::type;
  using brkex = parse_brkex<new_asts, list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle range expressions with escaped hyphens
 */
template <typename... asts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'\\'>, token<'-'>, tokens...>> {
  using brkex =
      parse_brkex<list<ast::symbol<'['>, asts...>, list<token<'-'>, tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle range expressions with escaped upper limits
 */
template <typename... asts, char c, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'-'>, token<'\\'>, token<c>, tokens...>> {
  using brkex = parse_brkex<list<ast::symbol<'['>, asts...>,
                            list<token<'-'>, token<c>, tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle range expressions with an escaped escape as its upper limit
 */
template <typename... asts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'-'>, token<'\\'>, token<'\\'>, tokens...>> {
  using init_asts = typename init<list<ast::symbol<'['>, asts...>>::type;
  using last_ast = typename last<list<asts...>>::type;
  using new_ast = typename ast::range<last_ast::value, '\\'>;
  using new_asts = typename append<init_asts, new_ast>::type;
  using brkex = parse_brkex<new_asts, list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle hyphen token ('-') at start of matching bracket expression
 */
template <typename... tokens>
struct parse_brkex<list<ast::symbol<'['>>, list<token<'-'>, tokens...>> {
  using brkex =
      parse_brkex<list<ast::symbol<'['>, ast::symbol<'-'>>, list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle hyphen token ('-') at start of non-matching bracket expression
 */
template <typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, ast::symbol<'^'>>,
                   list<token<'-'>, tokens...>> {
  using brkex =
      parse_brkex<list<ast::symbol<'['>, ast::symbol<'^'>, ast::symbol<'-'>>,
                  list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle hyphen token ('-') at end of bracket expression
 */
template <typename... asts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'-'>, token<']'>, tokens...>> {
  using brkex = parse_brkex<list<ast::symbol<'['>, asts..., ast::symbol<'-'>>,
                            list<token<']'>, tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle character class expressions
 */
template <typename... asts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'['>, token<':'>, tokens...>> {
  using claex = parse_ccex<list<>, list<token<'['>, token<':'>, tokens...>>;
  using brkex =
      parse_brkex<list<ast::symbol<'['>, asts..., typename claex::type>,
                  typename claex::unused>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle collating symbol expressions
 *
 * Note: The current implementation does not support collating symbols and
 *       instead reports and error
 */
template <typename... asts, char c0, char c1, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, asts...>,
                   list<token<'['>, token<'.'>, token<c0>, token<c1>,
                        token<'.'>, token<']'>, tokens...>>;

/**
 * Handle equivalence class expressions
 *
 * Note: The current implementation handles equivalence classes as individual
 *       tokens with extra syntax as locales are not involved
 */
template <typename... asts, char c, typename... tokens>
struct parse_brkex<
    list<ast::symbol<'['>, asts...>,
    list<token<'['>, token<'='>, token<c>, token<'='>, token<']'>, tokens...>> {

  using brkex = parse_brkex<list<ast::symbol<'['>, asts..., ast::symbol<c>>,
                            list<tokens...>>;
  using type = typename brkex::type;
  using unused = typename brkex::unused;
};

/**
 * Handle close-bracket character (']') for matching lists
 */
template <typename... parts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, parts...>,
                   list<token<']'>, tokens...>> {
  using type = ast::any_of<parts...>;
  using unused = list<tokens...>;
};

/**
 * Handle close-bracket character (']') for non-matching lists
 */
template <typename... parts, typename... tokens>
struct parse_brkex<list<ast::symbol<'['>, ast::symbol<'^'>, parts...>,
                   list<token<']'>, tokens...>> {
  using type = ast::none_of<parts...>;
  using unused = list<tokens...>;
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
 * Handle generic tokens
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
 * Handle asterisk character ('*') without preceding ast
 */
template <typename... tokens>
struct parse_regex<ast::sequence<>, list<token<'*'>, tokens...>> {
  using type = typename parse_regex<ast::sequence<ast::symbol<'*'>>,
                                    list<tokens...>>::type;
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
