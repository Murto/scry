#include "scry.hpp"

#include <cassert>
#include <cstdio>

constexpr static const char abcdef_pattern[] = R"(abcdef)";
constexpr static const char a____f_pattern[] = R"(a....f)";
constexpr static const char adotsf_pattern[] = R"(a\.\.\.\.f)";
constexpr static const char lotofa_pattern[] = R"(a*)";
constexpr static const char anchored_abcdef_pattern[] = R"(^abcdef$)";
constexpr static const char escaped_anchor_pattern[] = R"(^\^\^\$\$$)";

int main() {

  using abcdef = scry::regex<abcdef_pattern>;
  using a____f = scry::regex<a____f_pattern>;
  using adotsf = scry::regex<adotsf_pattern>;
  using lotofa = scry::regex<lotofa_pattern>;
  using anchored_abcdef = scry::regex<anchored_abcdef_pattern>;
  using escaped_anchor = scry::regex<escaped_anchor_pattern>;

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
}
