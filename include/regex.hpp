#pragma once

#include "ct_string.hpp"
#include "match.hpp"

namespace initregex {

template <const char *cs> class regex {

public:
  using string = ct_string<cs>;
};

} // namespace initregex
