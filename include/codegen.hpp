#pragma once

#include "parser.hpp"

#include <cstdio>

namespace scry {

namespace op {

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
 * TODO: Ensure nested `zero_or_more`s are handled properly (e.g. a****). This
 *       code assumes that they are never nested. There is a good case here for
 *       disallowing such a structure from being generated as it may simplify
 *       generated code.
 */
template <typename nested, typename next> struct accept_zero_or_more {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    maybe<it_type> best = next::execute(begin, end);
    while (begin != end) {
      if (auto it = nested::execute(begin, end)) {
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
template <std::size_t n, typename nested, typename next> struct accept_n {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
      if (auto it = nested::execute(begin, end)) {
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
template <std::size_t n, typename nested, typename next> struct accept_up_to_n {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    maybe<it_type> best = next::execute(begin, end);
    for (std::size_t i = 0; i < n; ++i) {
      if (auto it = nested::execute(begin, end)) {
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

} // namespace op

namespace {

template <typename ast> struct generate_code;

template <> struct generate_code<ast::sequence<>> { using type = op::noop; };

template <char c, typename... asts>
struct generate_code<ast::sequence<ast::symbol<c>, asts...>> {
  using type =
      typename op::accept<c,
                          typename generate_code<ast::sequence<asts...>>::type>;
};

template <typename... asts>
struct generate_code<ast::sequence<ast::any, asts...>> {
  using type = typename op::accept_any<
      typename generate_code<ast::sequence<asts...>>::type>;
};

template <typename nested, typename... asts>
struct generate_code<ast::sequence<ast::zero_or_more<nested>, asts...>> {
  using type = typename op::accept_zero_or_more<
      typename generate_code<nested>::type,
      typename generate_code<ast::sequence<asts...>>::type>;
};

template <typename... asts>
struct generate_code<ast::sequence<ast::left_anchor, asts...>> {
  using type = typename op::left_anchor<
      typename generate_code<ast::sequence<asts...>>::type>;
};

template <> struct generate_code<ast::sequence<ast::right_anchor>> {
  using type = op::right_anchor;
};

template <std::size_t n, typename nested, typename... asts>
struct generate_code<ast::sequence<ast::exactly<n, nested>, asts...>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      typename generate_code<ast::sequence<asts...>>::type>;
};

template <std::size_t n, typename nested, typename... asts>
struct generate_code<ast::sequence<ast::at_least<n, nested>, asts...>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      op::accept_zero_or_more<
          typename generate_code<nested>::type,
          typename generate_code<ast::sequence<asts...>>::type>>;
};

template <std::size_t n, std::size_t m, typename nested, typename... asts>
struct generate_code<ast::sequence<ast::between<n, m, nested>, asts...>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      op::accept_up_to_n<m - n, typename generate_code<nested>::type,
                         typename generate_code<ast::sequence<asts...>>::type>>;
};

template <char c> struct generate_code<ast::symbol<c>> {
  using type = typename op::accept<c, op::noop>;
};

template <> struct generate_code<ast::any> {
  using type = typename op::accept_any<op::noop>;
};

template <typename nested> struct generate_code<ast::zero_or_more<nested>> {
  using type =
      typename op::accept_zero_or_more<typename generate_code<nested>::type,
                                       op::noop>;
};

template <> struct generate_code<ast::left_anchor> {
  using type = typename op::left_anchor<op::noop>;
};

template <> struct generate_code<ast::right_anchor> {
  using type = op::right_anchor;
};

template <std::size_t n, typename nested>
struct generate_code<ast::exactly<n, nested>> {
  using type =
      typename op::accept_n<n, typename generate_code<nested>::type, op::noop>;
};

template <std::size_t n, typename nested>
struct generate_code<ast::at_least<n, nested>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      op::accept_zero_or_more<typename generate_code<nested>::type, op::noop>>;
};

template <std::size_t n, std::size_t m, typename nested>
struct generate_code<ast::between<n, m, nested>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      op::accept_up_to_n<m - n, typename generate_code<nested>::type,
                         op::noop>>;
};

} // anonymous namespace

template <typename ast> struct codegen_result {
  using type = typename generate_code<ast>::type;
};

} // namespace scry
