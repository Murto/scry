#pragma once

#include <cstddef>

namespace scry {

namespace {

constexpr std::size_t size_of_str(const char *cs) {
  std::size_t count = 0;
  while (*cs != '\0') {
    ++cs;
    ++count;
  }
  return count;
}

} // namespace

template <const char *cs> class ct_string {

public:
  using size_type = std::size_t;

  constexpr static char get(size_type index) { return cs[index]; }

  constexpr static size_type size = size_of_str(cs);
};

} // namespace scry
