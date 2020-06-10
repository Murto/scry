#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace initregex {

template <typename symbol_type> class nfa {

public:
  void add_initial_state(int state) noexcept { initial_states.insert(state); }
  void add_accepting_state(int state) noexcept {
    accepting_states.insert(state);
  }
  void add_transition(symbol_type symbol, int from, int to) noexcept {
    transitions[{symbol, from}] = to;
  }

private:
  struct pair_hash {

    template <typename type_1, typename type_2>
    std::size_t operator()(const std::pair<type_1, type_2> &key) const {
      auto h1 = std::hash<type_1>{}(key.first);
      auto h2 = std::hash<type_2>{}(key.second);
      return h1 ^ (h2 << 1);
    }
  };

  std::unordered_set<int> initial_states;
  std::unordered_set<int> accepting_states;
  std::unordered_map<std::pair<symbol_type, int>, int, pair_hash> transitions;
  std::unordered_set<int> states;
};

class regex {

public:
  regex(char *pattern);

  regex(const std::string &pattern);

  regex(std::string &&pattern);

private:
  nfa<char> accepter;
};

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

class match_results {

public:
  match_results() = delete;

  bool ready() noexcept;

  bool empty() noexcept;

  std::string::size_type length();

  std::string str() { return *match; }

private:
  bool is_ready;
  std::optional<std::string> match;
};

template <typename it_type>
bool regex_match(it_type begin, it_type end, regex r) {
  return false;
}

template <typename it_type>
bool regex_match(it_type begin, it_type end, match_results m, regex r) {
  return false;
}

bool regex_match(const std::string &s, regex r);

bool regex_match(const std::string &s, match_results m, regex r);

} // namespace initregex
