#include "scry.hpp"

#include <cassert>
#include <cstdio>

constexpr static const char abcdef_pattern[] = R"(abcdef)";
constexpr static const char a____f_pattern[] = R"(a....f)";
constexpr static const char adotsf_pattern[] = R"(a\.\.\.\.f)";
constexpr static const char lotofa_pattern[] = R"(a*)";
constexpr static const char anchored_abcdef_pattern[] = R"(^abcdef$)";
constexpr static const char escaped_anchor_pattern[] = R"(^\^\^\$\$$)";
constexpr static const char ten_as_pattern[] = R"(a\{10\})";
constexpr static const char least_ten_as_pattern[] = R"(a\{10,\})";
constexpr static const char between_as_pattern[] = R"(a\{5,10\})";

int main() {

  using abcdef = scry::regex<abcdef_pattern>;
  using a____f = scry::regex<a____f_pattern>;
  using adotsf = scry::regex<adotsf_pattern>;
  using lotofa = scry::regex<lotofa_pattern>;
  using anchored_abcdef = scry::regex<anchored_abcdef_pattern>;
  using escaped_anchor = scry::regex<escaped_anchor_pattern>;
  using ten_as = scry::regex<ten_as_pattern>;
  using least_ten_as = scry::regex<least_ten_as_pattern>;
  using between_as = scry::regex<between_as_pattern>;

  // Test char seqeuences
  assert(scry::regex_match<abcdef>("abcdef"));
  assert(!scry::regex_match<abcdef>(""));
  assert(!scry::regex_match<abcdef>("abcdefg"));
  assert(!scry::regex_match<abcdef>("fedcba"));

  // Test dot
  assert(scry::regex_match<a____f>("abcdef"));
  assert(!scry::regex_match<a____f>(""));
  assert(!scry::regex_match<a____f>("af"));
  assert(!scry::regex_match<a____f>("a??f"));

  // Test escaped dot
  assert(scry::regex_match<adotsf>("a....f"));
  assert(!scry::regex_match<adotsf>(""));
  assert(!scry::regex_match<adotsf>("abcdef"));

  // Test repeating character
  assert(scry::regex_match<lotofa>(""));
  assert(scry::regex_match<lotofa>("a"));
  assert(scry::regex_match<lotofa>("aa"));
  assert(scry::regex_match<lotofa>("aaaaaaaaaaaaaaaaaaaaaa"));
  assert(!scry::regex_match<lotofa>("ab"));
  assert(!scry::regex_match<lotofa>("aaaaaaaaaaaaaaaaaaaab"));
  assert(!scry::regex_match<lotofa>("ba"));
  assert(!scry::regex_match<lotofa>("baaaaaaaaaaaaaaaaaaaa"));

  // Test ignoring circumflexes and dollar-signs
  assert(scry::regex_match<anchored_abcdef>("abcdef"));
  assert(!scry::regex_match<anchored_abcdef>(""));
  assert(!scry::regex_match<anchored_abcdef>("^abcdef"));
  assert(!scry::regex_match<anchored_abcdef>("abcdef$"));
  assert(!scry::regex_match<anchored_abcdef>("^abcdef$"));

  // Test escaped circumflexes and dollar-signs
  assert(scry::regex_match<escaped_anchor>("^^$$"));
  assert(!scry::regex_match<escaped_anchor>(""));
  assert(!scry::regex_match<escaped_anchor>("^^^$$$"));

  // Test set intervals
  assert(scry::regex_match<ten_as>("aaaaaaaaaa"));
  assert(!scry::regex_match<ten_as>(""));
  assert(!scry::regex_match<ten_as>("aaaaaaaaa"));
  assert(!scry::regex_match<ten_as>("aaaaaaaaaaa"));

  // Test lower-bounded intervals
  assert(scry::regex_match<least_ten_as>("aaaaaaaaaa"));
  assert(!scry::regex_match<least_ten_as>(""));
  assert(!scry::regex_match<least_ten_as>("aaaaaaaaa"));
  assert(scry::regex_match<least_ten_as>("aaaaaaaaaaa"));
  assert(scry::regex_match<least_ten_as>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));

  // Test bounded intervals
  assert(scry::regex_match<between_as>("aaaaa"));
  assert(scry::regex_match<between_as>("aaaaaa"));
  assert(scry::regex_match<between_as>("aaaaaaa"));
  assert(scry::regex_match<between_as>("aaaaaaaa"));
  assert(scry::regex_match<between_as>("aaaaaaaaa"));
  assert(scry::regex_match<between_as>("aaaaaaaaaa"));
  assert(!scry::regex_match<between_as>(""));
  assert(!scry::regex_match<between_as>("aaaa"));
  assert(!scry::regex_match<between_as>("aaaaaaaaaaa"));
}
