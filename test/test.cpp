#include "initregex.hpp"

#include <iostream>

int main() {
  std::string input;
  std::cin >> input;
  initregex::regex regex(input);
}
