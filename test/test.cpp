#include "initregex.hpp"

#include <iostream>

int main() {
  std::string pattern, input;
  std::cout << "Enter a regular expression: ";
  std::cin >> pattern;
  std::cout << "Enter an input: ";
  std::cin >> input;
  initregex::regex regex(pattern);
  auto match = initregex::regex_match(input, regex);
  std::cout << (match ? "MATCH" : "NO MATCH") << '\n';
}
