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
    transitions[{symbol, from}].insert(to);
  }

  void add_transition(int from, int to) noexcept {
    epsilon_transitions[from].insert(to);
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
  std::unordered_map<std::pair<symbol_type, int>, std::unordered_set<int>,
                     pair_hash>
      transitions;
  std::unordered_map<int, std::unordered_set<int>> epsilon_transitions;
  std::unordered_set<int> states;
};

class regex {

public:
  regex(char *pattern);

  regex(const std::string &pattern);

  regex(std::string &&pattern);

private:
  nfa<char> acceptor;
};

class simple_expr {

  template <typename T> friend class nfa_visitor;

public:
  simple_expr(char c, bool dupl);

private:
  char c;
  bool dupl;

  template <typename visitor_type> auto accept(visitor_type &visitor) const {
    return visitor.visit(*this);
  }
};

class regex_expr {

  template <typename T> friend class nfa_visitor;

public:
  regex_expr(bool l_anchor, const std::vector<simple_expr> &exprs,
             bool r_anchor);

private:
  bool l_anchor;
  std::vector<simple_expr> exprs;
  bool r_anchor;

  template <typename visitor_type> auto accept(visitor_type &visitor) const {
    return visitor.visit(*this);
  }
};

template <typename symbol_type> class nfa_visitor {

public:
  nfa<symbol_type> acceptor;

  nfa_visitor() { acceptor.add_initial_state(next_id()); }

  void visit(const regex_expr &expr) {
    for (const auto &simple : expr.exprs) {
      simple.accept(*this);
    }
    next_id();
    acceptor.add_accepting_state(prev_id);
  }

  void visit(const simple_expr &expr) {
    int state_id = next_id();
    if (expr.dupl) {
      acceptor.add_transition(expr.c, prev_id, prev_id);
      acceptor.add_transition(prev_id, state_id);
    } else {
      acceptor.add_transition(expr.c, prev_id, state_id);
    }
  }

private:
  int id = 0;
  int prev_id;

  int next_id() noexcept {
    prev_id = id++;
    return id;
  }
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
