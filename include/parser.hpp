#pragma once

#include "lexer.hpp"
#include "util.hpp"

#include <cstdio>

namespace initregex {

template <typename it_type> struct operation_result {
  bool success;
  it_type it;
};

template <typename... operations> struct regex_matcher;

/**
 * Attempt match against input sequence
 */
template <typename operation, typename... operations>
struct regex_matcher<operation, operations...> {
  using next = regex_matcher<operations...>;
  template <typename it_type>
  constexpr static bool execute(it_type begin, it_type end) noexcept {
    auto result = operation::execute(begin, end);
    if (result.success) {
      return next::execute(result.it, end);
    } else {
      return false;
    }
  }
};

/**
 * Handle matching base case
 */
template <> struct regex_matcher<> {
  template <typename it_type>
  constexpr static bool execute(it_type begin, it_type end) noexcept {
    return begin == end;
  }
};

/**
 * Do nothing operation
 */
struct noop {
  template <typename it_type>
  constexpr static operation_result<it_type> execute(it_type begin,
                                                     it_type end) {
    return {true, begin};
  }
};

/**
 * Accept any character
 */
struct accept_any {
  template <typename it_type>
  constexpr static operation_result<it_type> execute(it_type begin,
                                                     it_type end) {
    if (begin != end) {
      return {true, begin + 1};
    } else {
      return {false, begin};
    }
  }
};

/**
 * Duplcate operation
 */
template <typename operation> struct duplicate {
  template <typename it_type>
  constexpr static operation_result<it_type> execute(it_type begin,
                                                     it_type end) {
    auto result = operation::execute(begin, end);
    auto it = begin;
    while (result.success) {
      it = result.it;
      result = operation::execute(result.it, end);
    }
    return {true, it};
  }
};

/**
 * Accept arbitrary sequence of characters
 */
template <char c> struct accept {
  template <typename it_type>
  constexpr static operation_result<it_type> execute(it_type begin,
                                                     it_type end) {
    if (begin != end && *begin == c) {
      return {true, begin + 1};
    } else {
      return {false, begin};
    }
  }
};

template <typename regex_matcher, typename... tokens> struct parse_matcher;

/**
 * Handle base case
 */
template <typename regex_matcher> struct parse_matcher<regex_matcher> {
  using type = regex_matcher;
};

/**
 * Handle left-anchor circumflex circumflexes
 */
template <typename... tokens>
struct parse_matcher<regex_matcher<>, circumflex, tokens...> {
  using type = typename parse_matcher<regex_matcher<>, tokens...>::type;
};

/**
 * Handle right-anchor dollar-signs
 */
template <typename... operations>
struct parse_matcher<regex_matcher<operations...>, dollar_sign> {
  using type = typename parse_matcher<regex_matcher<operations...>>::type;
};

/**
 * Handle dots
 */
template <typename... operations, typename... tokens>
struct parse_matcher<regex_matcher<operations...>, dot, tokens...> {
  using type = typename parse_matcher<regex_matcher<operations..., accept_any>,
                                      tokens...>::type;
};

/**
 * Handle asterisks with generic operations
 */
template <typename... operations, typename... tokens>
struct parse_matcher<regex_matcher<operations...>, asterisk, tokens...> {
  using type = typename parse_matcher<
      typename append<
          typename drop_right<regex_matcher<operations...>, 1>::type,
          duplicate<typename last<regex_matcher<operations...>>::type>>::type,
      tokens...>::type;
};

/**
 * Handle symbols
 */
template <typename... operations, char c, typename... tokens>
struct parse_matcher<regex_matcher<operations...>, symbol<c>, tokens...> {
  using type = typename parse_matcher<regex_matcher<operations..., accept<c>>,
                                      tokens...>::type;
};

template <typename list> struct parse_result;

template <typename... tokens> struct parse_result<list<tokens...>> {
  using matcher = typename parse_matcher<regex_matcher<>, tokens...>::type;
};

} // namespace initregex
