#include "initregex.hpp"

namespace initregex {

regex::regex(char *pattern) : pattern{pattern} {}

regex::regex(const std::string &pattern) : pattern{pattern} {}

regex::regex(std::string &&pattern) : pattern{std::move(pattern)} {}

} // namespace initregex
