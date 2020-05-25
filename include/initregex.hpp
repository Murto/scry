#pragma once

#include <string>
#include <vector>

namespace initregex {

class regex {

public:
  regex(char *pattern);

  regex(const std::string &pattern);

  regex(std::string &&pattern);

private:
  class simple_expr {

  public:
    simple_expr(char c, bool dupl);

  private:
    char c;
    bool dupl;
  };

  class regex_expr {

  public:
    regex_expr(bool l_anchor, const std::vector<simple_expr> &exprs,
               bool r_anchor);

  private:
    bool l_anchor;
    std::vector<simple_expr> exprs;
    bool r_anchor;
  };

  regex_expr expr;
};

} // namespace initregex
