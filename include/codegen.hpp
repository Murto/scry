#pragma once

#include "parser.hpp"

#include <cstdio>

namespace scry {

namespace op {

/**
 * Structure containing a sequence of operations which must be accepted in
 * sequence
 */
template <typename... ops> struct accept_sequence;

template <typename head, typename... tail>
struct accept_sequence<head, tail...> {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (auto it = head::execute(begin, end)) {
      return accept_sequence<tail...>::execute(*it, end);
    } else {
      return {};
    }
  }
};

template <> struct accept_sequence<> {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type>
  execute(it_type begin, [[maybe_unused]] it_type end) noexcept {
    return begin;
  }
};

/**
 * Structure representing generic symbols
 */
template <char c> struct accept {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && *begin == c) {
      return ++begin;
    } else {
      return {};
    }
  }
};

template <char c> struct reject {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && *begin != c) {
      return ++begin;
    } else {
      return {};
    }
  }
};

/**
 * Structure representing the any character (.)
 */
struct accept_any {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end) {
      return ++begin;
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
 * Specialization of `accept_zero_or_more` for cases where there is not a
 * next operation
 */
template <typename nested>
struct accept_zero_or_more<nested, op::accept_sequence<>> {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    while (begin != end) {
      if (auto it = nested::execute(begin, end)) {
        begin = it;
      } else {
        return {};
      }
    }
    return begin;
  }
};

/**
 * Specialization of `accept_zero_or_more` for cases where a single symbol
 * follows
 */
template <typename nested, typename until, typename next> struct accept_until {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    while (begin != end) {
      if (auto it = until::execute(begin, end)) {
        return next::execute(*it, end);
      } else if (auto it = nested::execute(begin, end)) {
        begin = it;
      } else {
        return {};
      }
    }
    return begin;
  }
};

/**
 * Structure representing the left-anchor (^)
 *
 * TODO: Figure out if this is fine as is, or whether there is some meaningful
 *       code that can be inserted in here. Maybe parameterise execute with a
 *       begin, current, and end iterator?
 */
struct left_anchor {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type>
  execute(it_type begin, [[maybe_unused]] it_type end) noexcept {
    return begin;
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
template <std::size_t n, typename nested> struct accept_n {
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
    return begin;
  }
};

/**
 * Structure representing an upper-bounded repetition of an AST
 */
template <std::size_t n, typename nested, typename next> struct accept_at_most {
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
template <char lower, char upper> struct accept_range {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && ((lower <= *begin) & (*begin <= upper))) {
      return ++begin;
    } else {
      return {};
    }
  }
};

/**
 * Structure representing a range of symbols which may be rejected
 */
template <char lower, char upper> struct reject_range {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && ((lower > *begin) | (*begin > upper))) {
      return ++begin;
    } else {
      return {};
    }
  }
};

template <typename nested> struct op_if {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    if (begin != end && nested::execute(*begin)) {
      return ++begin;
    } else {
      return {};
    }
  }
};

} // namespace op

namespace pred {

template <char c0> struct equals {
  SCRY_INLINE constexpr static bool execute(char c1) noexcept {
    return c0 == c1;
  }
};

template <char lower, char upper> struct in_range {
  SCRY_INLINE constexpr static bool execute(char c) noexcept {
    return (lower <= c) & (c <= upper);
  }
};

template <typename nested> struct negate {
  SCRY_INLINE constexpr static bool execute(char c) noexcept {
    return !nested::execute(c);
  }
};

template <typename left, typename right> struct left_or_right {
  SCRY_INLINE constexpr static bool execute(char c) noexcept {
    return left::execute(c) | right::execute(c);
  }
};

template <typename left, typename right> struct left_and_right {
  SCRY_INLINE constexpr static bool execute(char c) noexcept {
    return left::execute(c) & right::execute(c);
  }
};

} // namespace pred

namespace {

template <typename op, typename ast> struct generate_ops;

template <typename ast> struct generate_op;

template <typename ast> struct generate_pred;

template <typename op> struct generate_ops<op, ast::sequence<>> {
  using type = op;
};

template <typename... ops, typename head, typename... tail>
struct generate_ops<op::accept_sequence<ops...>, ast::sequence<head, tail...>> {
  using type = typename generate_ops<
      op::accept_sequence<ops..., typename generate_op<head>::type>,
      ast::sequence<tail...>>::type;
};

template <typename... ops, typename nested, typename... asts>
struct generate_ops<op::accept_sequence<ops...>,
                    ast::sequence<ast::zero_or_more<nested>, asts...>> {
  using nested_op = typename generate_op<nested>::type;
  using next_op = typename generate_ops<op::accept_sequence<>,
                                        ast::sequence<asts...>>::type;
  using type =
      op::accept_sequence<ops...,
                          typename op::accept_zero_or_more<nested_op, next_op>>;
};

template <typename... ops, std::size_t n, typename nested, typename... asts>
struct generate_ops<op::accept_sequence<ops...>,
                    ast::sequence<ast::at_least<n, nested>, asts...>> {
  using nested_op = typename generate_op<nested>::type;
  using next_op = typename generate_ops<op::accept_sequence<>,
                                        ast::sequence<asts...>>::type;
  using type =
      typename op::accept_sequence<ops..., op::accept_n<n, nested_op>,
                                   op::accept_zero_or_more<nested_op, next_op>>;
};

template <typename... ops, std::size_t n, typename nested, typename... asts>
struct generate_ops<op::accept_sequence<ops...>,
                    ast::sequence<ast::at_most<n, nested>, asts...>> {
  using nested_op = typename generate_op<nested>::type;
  using next_op = typename generate_ops<op::accept_sequence<>,
                                        ast::sequence<asts...>>::type;
  using type =
      typename op::accept_sequence<ops...,
                                   op::accept_at_most<n, nested_op, next_op>>;
};

template <char c> struct generate_op<ast::symbol<c>> {
  using type = typename op::accept<c>;
};

template <> struct generate_op<ast::any> { using type = op::accept_any; };

template <> struct generate_op<ast::left_anchor> {
  using type = op::left_anchor;
};

template <> struct generate_op<ast::right_anchor> {
  using type = op::right_anchor;
};

template <std::size_t n, typename nested>
struct generate_op<ast::exactly<n, nested>> {
  using type = typename op::accept_n<n, typename generate_op<nested>::type>;
};

template <char lower, char upper> struct generate_op<ast::range<lower, upper>> {
  using type = op::accept_range<lower, upper>;
};

template <typename... nested> struct generate_op<ast::any_of<nested...>> {
  using type = op::op_if<typename generate_pred<ast::any_of<nested...>>::type>;
};

template <typename... nested> struct generate_op<ast::none_of<nested...>> {
  using type = op::op_if<typename generate_pred<ast::none_of<nested...>>::type>;
};

template <char c> struct generate_pred<ast::symbol<c>> {
  using type = pred::equals<c>;
};

template <char lower, char upper>
struct generate_pred<ast::range<lower, upper>> {
  using type = pred::in_range<lower, upper>;
};

template <typename head, typename... tail>
struct generate_pred<ast::any_of<head, tail...>> {
  using left = typename generate_pred<head>::type;
  using right = typename generate_pred<ast::any_of<tail...>>::type;
  using type = pred::left_or_right<left, right>;
};

template <typename head> struct generate_pred<ast::any_of<head>> {
  using type = typename generate_pred<head>::type;
};

template <typename... nested> struct generate_pred<ast::none_of<nested...>> {
  using type = typename pred::negate<
      typename generate_pred<ast::any_of<nested...>>::type>;
};

} // anonymous namespace

template <typename ast> struct codegen_result {
  using type = typename generate_ops<op::accept_sequence<>, ast>::type;
};

} // namespace scry
