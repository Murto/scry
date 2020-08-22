#pragma once

#include "definitions.hpp"
#include "parser.hpp"

#include <utility>

namespace scry {

/**
 * Helper implementation of optional type
 * Note: std::optional is not used as the library aims to move its
 *       compatibility backwards towards c++11 (though a performance comparison
 *       would be interesting).
 */
template <typename type> struct maybe {
  type value{};
  bool some{false};
  SCRY_INLINE constexpr maybe() noexcept = default;
  SCRY_INLINE constexpr maybe(const type &value) noexcept
      : value{value}, some{true} {}
  SCRY_INLINE constexpr maybe(type &&value) noexcept
      : value{std::move(value)}, some{true} {}
  SCRY_INLINE constexpr maybe(const maybe<type> &other) noexcept
      : value{other.value}, some{other.some} {}
  SCRY_INLINE constexpr maybe(maybe<type> &&other) noexcept
      : value{std::move(other.value)}, some{other.some} {}
  SCRY_INLINE constexpr operator bool() const noexcept { return some; }
  SCRY_INLINE constexpr operator type() const noexcept { return value; }
  SCRY_INLINE constexpr maybe<type> &
  operator=(const maybe<type> &other) noexcept {
    value = other.value;
    some = other.some;
    return *this;
  }
  SCRY_INLINE constexpr maybe<type> &operator=(maybe<type> &&other) noexcept {
    value = std::move(other.value);
    some = other.some;
    return *this;
  }
};

namespace op {

struct noop {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type>
  execute(it_type begin, [[maybe_unused]] it_type end) noexcept {
    return begin;
  }
};

template <typename next, typename op> struct none_or_more {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    auto it = begin;
    maybe<it_type> best;
    while (true) {
      if (auto next_it = next::execute(it, end)) {
        best = next_it;
      }
      auto next_it = op::execute(it, end);
      if (!next_it) {
        return best;
      }
      it = next_it;
    }
  }
};

template <typename next, typename op, std::size_t n> struct repeat {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    auto it = begin;
    for (std::size_t i = 0; i < n; ++i) {
      auto next_it = op::execute(it, end);
      if (!next_it) {
        return {};
      }
      it = next_it;
    }
    return next::execute(it, end);
  }
};

template <typename next, typename op, std::size_t n> struct at_least {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    auto it = begin;
    for (std::size_t i = 0; i < n; ++i) {
      auto next_it = op::execute(it, end);
      if (!next_it) {
        return {};
      }
      it = next_it;
    }
    maybe<it_type> best = it;
    while (true) {
      if (auto next_it = next::execute(it, end)) {
        best = next_it;
      }
      auto next_it = op::execute(it, end);
      if (!next_it) {
        return best;
      }
      it = next_it;
    }
  }
};

template <typename next, typename op, std::size_t n, std::size_t m>
struct between {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    auto it = begin;
    for (std::size_t i = 0; i < n; ++i) {
      auto next_it = op::execute(it, end);
      if (!next_it) {
        return {};
      }
      it = next_it;
    }
    maybe<it_type> best = it;
    for (std::size_t i = 0; i < m - n; ++i) {
      if (auto next_it = next::execute(it, end)) {
        best = next_it;
      }
      auto next_it = op::execute(it, end);
      if (!next_it) {
        return best;
      }
      it = next_it;
    }
    if (auto next_it = next::execute(it, end)) {
      best = next_it;
    }
    return best;
  }
};

template <typename next> struct left_anchor {
  template <typename it_type>
  SCRY_INLINE constexpr static maybe<it_type> execute(it_type begin,
                                                      it_type end) noexcept {
    return next::execute(begin, end);
  }
};

template <typename next> struct right_anchor {
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

template <typename next> struct any {
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

template <typename next, char c> struct symbol {
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

} // namespace op

template <typename ast> struct generation;

template <char c> struct generation<ast::symbol<c>> {
  using type = op::symbol<op::noop, c>;
};

template <> struct generation<ast::sequence<>> { using type = op::noop; };

template <typename inner_ast, typename... asts>
struct generation<ast::sequence<ast::none_or_more<inner_ast>, asts...>> {
  using inner = typename generation<inner_ast>::type;
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::none_or_more<next, inner>;
};

template <typename inner_ast, std::size_t n, typename... asts>
struct generation<ast::sequence<ast::interval<inner_ast, n>, asts...>> {
  using inner = typename generation<inner_ast>::type;
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::repeat<next, inner, n>;
};

template <typename inner_ast, std::size_t n, typename... asts>
struct generation<ast::sequence<ast::lower_bound<inner_ast, n>, asts...>> {
  using inner = typename generation<inner_ast>::type;
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::at_least<next, inner, n>;
};

template <typename inner_ast, std::size_t n, std::size_t m, typename... asts>
struct generation<ast::sequence<ast::bounded<inner_ast, n, m>, asts...>> {
  using inner = typename generation<inner_ast>::type;
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::between<next, inner, n, m>;
};

template <typename... asts>
struct generation<ast::sequence<ast::left_anchor, asts...>> {
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::left_anchor<next>;
};

template <typename... asts>
struct generation<ast::sequence<ast::right_anchor, asts...>> {
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::right_anchor<next>;
};

template <typename... asts>
struct generation<ast::sequence<ast::any, asts...>> {
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::any<next>;
};

template <char c, typename... asts>
struct generation<ast::sequence<ast::symbol<c>, asts...>> {
  using next = typename generation<ast::sequence<asts...>>::type;
  using type = typename op::symbol<next, c>;
};

template <typename ast> struct generation_result {
  using type = typename generation<ast>::type;
};

} // namespace scry
