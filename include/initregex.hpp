#pragma once

#include <string>

namespace initregex {

class regex {

public:
  regex(char *pattern);

  regex(const std::string &pattern);

  regex(std::string &&pattern);

private:
  std::string pattern;
};

} // namespace initregex
