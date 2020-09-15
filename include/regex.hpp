#pragma once

#include "ct_string.hpp"
#include "traits.hpp"

namespace scry {

template <const char *cs, const trait_type traits = trait::basic> class regex {

public:
  using string = ct_string<cs>;
};

} // namespace scry
