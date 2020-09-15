#pragma once

#include "parser.hpp"
#include "util.hpp"

#include <type_traits>

namespace scry {

namespace ast {

template <typename nested, typename next> struct until;

} // namespace ast

namespace {

template <typename ast> struct optimise { using type = ast; };

template <typename head, typename... tail>
struct optimise<ast::sequence<head, tail...>> {
  using type = typename prepend<typename optimise<ast::sequence<tail...>>::type,
                                head>::type;
};

/**
 * Transforms regex of the form "A*A" into "AA*". This reduces the amount of
 * backtracking in cases where "A*A" comes at the end of the regex.
 */
template <typename head, typename... tail>
struct optimise<ast::sequence<ast::zero_or_more<head>, head, tail...>> {
  using type = typename optimise<
      ast::sequence<head, ast::zero_or_more<head>, tail...>>::type;
};

/**
 * Flattens `ast::sequence`s into a single `ast::sequence`
 */
template <typename... nested, typename... tail>
struct optimise<ast::sequence<ast::sequence<nested...>, tail...>> {
  using type = typename optimise<ast::sequence<nested..., tail...>>::type;
};

} // anonymous namespace

template <typename ast> struct optimise_result {
  using type = typename optimise<ast>::type;
};

} // namespace  scry
