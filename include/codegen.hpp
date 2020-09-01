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
template <char c, typename next = noop> struct accept {
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

template <char c, typename next = noop> struct reject {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && *begin != c) {
      return next::execute(begin + 1, end);
    } else {
      return {};
    }
  }
};

/**
 * Structure representing the any character (.)
 */
template <typename next = noop> struct accept_any {
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
template <typename nested, typename next = noop> struct accept_zero_or_more {
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
template <typename next = noop> struct left_anchor {
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
template <std::size_t n, typename nested, typename next = noop>
struct accept_n {
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
template <std::size_t n, typename nested, typename next = noop>
struct accept_up_to_n {
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

/**
 * Structure representing a range of symbols which may be accepted
 */
template <char lower, char upper, typename next = noop> struct accept_range {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && lower <= *begin && *begin <= upper) {
      return next::execute(begin + 1, end);
    } else {
      return {};
    }
  }
};

/**
 * Structure representing a range of symbols which may be rejected
 */
template <char lower, char upper, typename next = noop> struct reject_range {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && (lower > *begin || *begin > upper)) {
      return next::execute(begin + 1, end);
    } else {
      return {};
    }
  }
};

template <typename nested, typename next = noop> struct op_if {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (auto it = nested::execute(begin, end)) {
      return next::execute(*it, end);
    } else {
      return {};
    }
  }
};

template <typename left, typename right> struct op_or {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    auto left_it = left::execute(begin, end);
    auto right_it = right::execute(begin, end);
    if (left_it && right_it) {
      if (left_it > right_it) {
        return left_it;
      } else {
        return right_it;
      }
    } else if (left_it) {
      return left_it;
    } else if (right_it) {
      return right_it;
    } else {
      return {};
    }
  }
};

template <typename left, typename right> struct op_and {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    auto left_it = left::execute(begin, end);
    auto right_it = right::execute(begin, end);
    if (left_it && right_it) {
      if (left_it > right_it) {
        return left_it;
      } else {
        return right_it;
      }
    } else {
      return {};
    }
  }
};

} // namespace op

namespace {

template <typename op> struct negate;

template <> struct negate<op::noop> { using type = op::noop; };

template <char c, typename next> struct negate<op::accept<c, next>> {
  using type = op::reject<c, next>;
};

template <char c, typename next> struct negate<op::reject<c, next>> {
  using type = op::accept<c, next>;
};

template <char lower, char upper, typename next>
struct negate<op::accept_range<lower, upper, next>> {
  using type = op::reject_range<lower, upper, next>;
};

template <char lower, char upper, typename next>
struct negate<op::reject_range<lower, upper, next>> {
  using type = op::accept_range<lower, upper, next>;
};

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

template <typename... nested, typename... asts>
struct generate_code<ast::sequence<ast::any_of<nested...>, asts...>> {
  using type = op::op_if<typename generate_code<ast::any_of<nested...>>::type,
                         typename generate_code<asts...>::type>;
};

template <char c> struct generate_code<ast::symbol<c>> {
  using type = typename op::accept<c>;
};

template <> struct generate_code<ast::any> {
  using type = typename op::accept_any<>;
};

template <typename nested> struct generate_code<ast::zero_or_more<nested>> {
  using type =
      typename op::accept_zero_or_more<typename generate_code<nested>::type>;
};

template <> struct generate_code<ast::left_anchor> {
  using type = typename op::left_anchor<>;
};

template <> struct generate_code<ast::right_anchor> {
  using type = op::right_anchor;
};

template <std::size_t n, typename nested>
struct generate_code<ast::exactly<n, nested>> {
  using type = typename op::accept_n<n, typename generate_code<nested>::type>;
};

template <std::size_t n, typename nested>
struct generate_code<ast::at_least<n, nested>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      op::accept_zero_or_more<typename generate_code<nested>::type>>;
};

template <std::size_t n, std::size_t m, typename nested>
struct generate_code<ast::between<n, m, nested>> {
  using type = typename op::accept_n<
      n, typename generate_code<nested>::type,
      op::accept_up_to_n<m - n, typename generate_code<nested>::type>>;
};

template <char lower, char upper>
struct generate_code<ast::range<lower, upper>> {
  using type = op::accept_range<lower, upper>;
};

template <typename nested> struct generate_code<ast::any_of<nested>> {
  using type = typename generate_code<nested>::type;
};

template <typename head, typename... tail>
struct generate_code<ast::any_of<head, tail...>> {
  using left = typename generate_code<head>::type;
  using right = typename generate_code<ast::any_of<tail...>>::type;
  using type = op::op_or<left, right>;
};

template <typename nested> struct generate_code<ast::none_of<nested>> {
  using type = typename negate<typename generate_code<nested>::type>::type;
};

template <typename head, typename... tail>
struct generate_code<ast::none_of<head, tail...>> {
  using left = typename negate<typename generate_code<head>::type>::type;
  using right = typename generate_code<ast::none_of<tail...>>::type;
  using type = op::op_and<left, right>;
};

} // anonymous namespace

template <typename ast> struct codegen_result {
  using type = typename generate_code<ast>::type;
};

} // namespace scry
