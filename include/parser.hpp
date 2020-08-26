#pragma once

#include "definitions.hpp"
#include "util.hpp"

namespace scry {

namespace ast {

/**
 * Structure representing an empty regex and leaf for an ast
 */
struct noop {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type>
  execute(it_type begin, [[maybe_unused]] it_type end) noexcept {
    return begin;
  }
};

/**
 * Structure representing generic symbols
 */
template <char c, typename next> struct accept {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && *begin == c) {
      return next::execute(begin + 1, end);
    } else {
      return {};
    }
  }
};

/**
 * Structure representing the any character (.)
 */
template <typename next> struct accept_any {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end) {
      return next::execute(begin + 1, end);
    } else {
      return {};
    }
  }
};

/**
 * Structure representing the none-or-more operation (*)
 *
 * TODO: Ensure nested `none_or_more`s are handled properly (e.g. a****). This
 *       code assumes that they are never nested. There is a good case here for
 *       disallowing such a structure from being generated as it may simplify
 *       generated code.
 */
template <typename prev, typename next> struct none_or_more {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    maybe<it_type> best = next::execute(begin, end);
    while (begin != end) {
      if (auto it = prev::execute(begin, end)) {
        begin = it;
      } else {
        return best;
      }
      if (auto it = next::execute(begin, end)) {
        best = it;
      }
    }
    return best;
  }
};

/**
 * Structure representing the left-anchor (^)
 *
 * TODO: Figure out if this is fine as is, or whether there is some meaningful
 *       code that can be inserted in here. Maybe parameterise execute with a
 *       begin, current, and end iterator?
 */
template <typename next> struct left_anchor {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    return next::execute(begin, end);
  }
};

/**
 * Structure representing the right-anchor ($)
 */
struct right_anchor {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin == end) {
      return begin;
    } else {
      return {};
    }
  }
};

/**
 * Structure representing the repetition of an AST
 */
template <std::size_t n, typename prev, typename next> struct repeat {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
      if (auto it = prev::execute(begin, end)) {
        begin = it;
      } else {
        return {};
      }
    }
    return next::execute(begin, end);
  }
};

/**
 * Structure representing an upper-bounded repetition of an AST
 */
template <std::size_t n, typename prev, typename next> struct bounded_repeat {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    maybe<it_type> best = next::execute(begin, end);
    for (std::size_t i = 0; i < n; ++i) {
      if (auto it = prev::execute(begin, end)) {
        begin = it;
      } else {
        break;
      }
      if (auto it = next::execute(begin, end)) {
        best = it;
      }
    }
    return best;
  }
};

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
template <char c> struct symbol;

/**
 * Helper struct to convert number values into types
 */
template <std::size_t n> struct number;

/**
 * Structure used for parsing brace expressions
 */
template <typename parts, typename tokens> struct parse_brcex;

/**
 * Structure used for parsing regular expressions
 */
template <typename tokens> struct parse_regex;

/**
 * Structure used for parsing suffixes
 */
template <template <typename> typename param_type, typename tokens>
struct parse_suffix;

/**
 * Handle open-brace digraph ('\{')
 */
template <typename... tokens>
struct parse_brcex<list<>, list<symbol<'\\'>, symbol<'{'>, tokens...>> {
  using brcex = parse_brcex<list<symbol<'{'>>, list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};

/**
 * Handle lower bound number
 */
template <char c, typename... tokens>
struct parse_brcex<list<symbol<'{'>>, list<symbol<c>, tokens...>> {
  using brcex = parse_brcex<list<symbol<'{'>, number<to_digit<c>::value>>,
                            list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};

template <std::size_t n, char c, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>>, list<symbol<c>, tokens...>> {
  using brcex =
      parse_brcex<list<symbol<'{'>, number<n * 10 + to_digit<c>::value>>,
                  list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};

template <char c, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<0>>, list<symbol<c>, tokens...>>;

/**
 * Handle comma character (',')
 */
template <std::size_t n, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>>, list<symbol<','>, tokens...>> {
  using brcex =
      parse_brcex<list<symbol<'{'>, number<n>, symbol<','>>, list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};

/**
 * Handle upper bound number
 */
template <std::size_t n, char c, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<','>>,
                   list<symbol<c>, tokens...>> {
  using brcex = parse_brcex<
      list<symbol<'{'>, number<n>, symbol<','>, number<to_digit<c>::value>>,
      list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};
template <std::size_t n, size_t m, char c, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<','>, number<m>>,
                   list<symbol<c>, tokens...>> {
  using brcex = parse_brcex<list<symbol<'{'>, number<n>, symbol<','>,
                                 number<m * 10 + to_digit<c>::value>>,
                            list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};
template <std::size_t n, char c, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<','>, number<0>>,
                   list<symbol<c>, tokens...>>;

/**
 * Handle cases for closing digraph ('\}')
 */
template <std::size_t n, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>>,
                   list<symbol<'\\'>, symbol<'}'>, tokens...>> {
  using brcex =
      parse_brcex<list<symbol<'{'>, number<n>, symbol<'}'>>, list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};

template <std::size_t n, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<','>>,
                   list<symbol<'\\'>, symbol<'}'>, tokens...>> {
  using brcex =
      parse_brcex<list<symbol<'{'>, number<n>, symbol<','>, symbol<'}'>>,
                  list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
  using unused = typename brcex::unused;
};

template <std::size_t n, std::size_t m, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<','>, number<m>>,
                   list<symbol<'\\'>, symbol<'}'>, tokens...>> {
  using brcex = parse_brcex<
      list<symbol<'{'>, number<n>, symbol<','>, number<m>, symbol<'}'>>,
      list<tokens...>>;
  template <typename prev, typename next>
  using type = typename brcex::template type<prev, next>;
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
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<'}'>>, list<tokens...>> {
  template <typename prev, typename next>
  using type = ast::repeat<n, prev, next>;
  using unused = list<tokens...>;
};

template <std::size_t n, typename... tokens>
struct parse_brcex<list<symbol<'{'>, number<n>, symbol<','>, symbol<'}'>>,
                   list<tokens...>> {
  template <typename prev, typename next>
  using type = ast::repeat<n, prev, ast::none_or_more<prev, next>>;
  using unused = list<tokens...>;
};

template <std::size_t n, std::size_t m, typename... tokens>
struct parse_brcex<
    list<symbol<'{'>, number<n>, symbol<','>, number<m>, symbol<'}'>>,
    list<tokens...>> {
  template <typename prev, typename next>
  using type = ast::repeat<n, prev, ast::bounded_repeat<m - n, prev, next>>;
  using unused = list<tokens...>;
};

/**
 * Structure used for parsing suffixes
 */
template <template <typename> typename param_type, typename tokens>
struct parse_suffix {
  using type = param_type<typename parse_regex<tokens>::type>;
};

/**
 * Handle asterisk character ('*')
 */
template <template <typename> typename param_type, typename... tokens>
struct parse_suffix<param_type, list<symbol<'*'>, tokens...>> {
  template <typename next>
  using new_param_type =
      typename ast::template none_or_more<param_type<ast::noop>, next>;
  using type = typename parse_suffix<new_param_type, list<tokens...>>::type;
};

/**
 * Handle open brace digraph ('\{')
 */
template <template <typename> typename param_type, typename... tokens>
struct parse_suffix<param_type, list<symbol<'\\'>, symbol<'{'>, tokens...>> {
  using brcex = parse_brcex<list<>, list<symbol<'\\'>, symbol<'{'>, tokens...>>;
  template <typename next>
  using new_param_type =
      typename brcex::template type<param_type<ast::noop>, next>;
  using type =
      typename parse_suffix<new_param_type, typename brcex::unused>::type;
};

/**
 * Handle end of input
 */
template <> struct parse_regex<list<>> { using type = ast::noop; };

/**
 * Handle generic characters
 */
template <char c, typename... tokens>
struct parse_regex<list<symbol<c>, tokens...>> {
  template <typename next> using param_type = ast::accept<c, next>;
  using type = typename parse_suffix<param_type, list<tokens...>>::type;
};

/**
 * Handle dot character ('.')
 */
template <typename... tokens> struct parse_regex<list<symbol<'.'>, tokens...>> {
  using type = typename parse_suffix<ast::accept_any, list<tokens...>>::type;
};

/**
 * Handle escaped dot character ('\.')
 */
template <typename... tokens>
struct parse_regex<list<symbol<'\\'>, symbol<'.'>, tokens...>> {
  template <typename next> using param_type = ast::accept<'.', next>;
  using type = typename parse_suffix<param_type, list<tokens...>>::type;
};

/**
 * Handle escaped escape character ('\\')
 */
template <typename... tokens>
struct parse_regex<list<symbol<'\\'>, symbol<'\\'>, tokens...>> {
  template <typename next> using param_type = ast::accept<'\\', next>;
  using type = typename parse_suffix<param_type, list<tokens...>>::type;
};

/**
 * Disallow invalid escape character ('\')
 */
template <typename... tokens> struct parse_regex<list<symbol<'\\'>, tokens...>>;

/**
 * Handle circumflex character ('^')
 */
template <typename... tokens> struct parse_regex<list<symbol<'^'>, tokens...>> {
  using type = typename parse_suffix<ast::left_anchor, list<tokens...>>::type;
};

/**
 * Handle escaped circumflex character ('\^')
 */
template <typename... tokens>
struct parse_regex<list<symbol<'\\'>, symbol<'^'>, tokens...>> {
  template <typename next> using param_type = ast::accept<'^', next>;
  using type = typename parse_suffix<param_type, list<tokens...>>::type;
};

/**
 * Handle dollar-sign character ('$')
 */
template <> struct parse_regex<list<symbol<'$'>>> {
  using type = ast::right_anchor;
};

/**
 * Handle escaped dollar-sign character ('$')
 */
template <typename... tokens>
struct parse_regex<list<symbol<'\\'>, symbol<'$'>, tokens...>> {
  template <typename next> using param_type = ast::accept<'$', next>;
  using type = typename parse_suffix<param_type, list<tokens...>>::type;
};

} // anonymous namespace

/**
 * Top level structure for parsing regular expressions
 */
template <typename regex, typename sequence> struct parse_result;

template <typename regex, std::size_t... n>
struct parse_result<regex, std::index_sequence<n...>> {
  using type =
      typename parse_regex<list<symbol<regex::string::get(n)>...>>::type;
};

} // namespace scry
