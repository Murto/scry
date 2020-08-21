#pragma once

#include "parser.hpp"

namespace scry {

struct noop {
  template <typename it_type>
  constexpr static bool execute(it_type begin, it_type end) {
    return false;
  }
};

template <typename tree> struct generation { using type = noop; };

template <typename tree> struct generation_result {
  using type = typename generation<tree>::type;
};

} // namespace scry
