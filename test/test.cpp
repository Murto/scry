#include "initregex.hpp"

#include <cassert>
#include <cstdio>

constexpr static const char abcdef_pattern[] = R"(abcdef)";
constexpr static const char a____f_pattern[] = R"(a....f)";
constexpr static const char adotsf_pattern[] = R"(a\.\.\.\.f)";
constexpr static const char lotofa_pattern[] = R"(a*)";
constexpr static const char anchored_abcdef_pattern[] = R"(^abcdef$)";
constexpr static const char escaped_anchor_pattern[] = R"(^\^\^\$\$$)";

int main() {

  using abcdef = initregex::regex<abcdef_pattern>;
  using a____f = initregex::regex<a____f_pattern>;
  using adotsf = initregex::regex<adotsf_pattern>;
  using lotofa = initregex::regex<lotofa_pattern>;
  using anchored_abcdef = initregex::regex<anchored_abcdef_pattern>;
  using escaped_anchor = initregex::regex<escaped_anchor_pattern>;

  // Test char seqeuences
  assert(initregex::regex_match<abcdef>("abcdef"));
  assert(!initregex::regex_match<abcdef>(""));
  assert(!initregex::regex_match<abcdef>("abcdefg"));
  assert(!initregex::regex_match<abcdef>("fedcba"));

  // Test dot
  assert(initregex::regex_match<a____f>("abcdef"));
  assert(!initregex::regex_match<a____f>(""));
  assert(!initregex::regex_match<a____f>("af"));
  assert(!initregex::regex_match<a____f>("a??f"));

  // Test escaped dot
  assert(initregex::regex_match<adotsf>("a....f"));
  assert(!initregex::regex_match<adotsf>(""));
  assert(!initregex::regex_match<adotsf>("abcdef"));

  // Test repeating character
  assert(initregex::regex_match<lotofa>(""));
  assert(initregex::regex_match<lotofa>("a"));
  assert(initregex::regex_match<lotofa>("aa"));
  assert(initregex::regex_match<lotofa>("aaaaaaaaaaaaaaaaaaaaaa"));
  assert(!initregex::regex_match<lotofa>("ab"));
  assert(!initregex::regex_match<lotofa>("aaaaaaaaaaaaaaaaaaaab"));
  assert(!initregex::regex_match<lotofa>("ba"));
  assert(!initregex::regex_match<lotofa>("baaaaaaaaaaaaaaaaaaaa"));

  // Test ignoring circumflexes and dollar-signs
  assert(initregex::regex_match<anchored_abcdef>("abcdef"));
  assert(!initregex::regex_match<anchored_abcdef>(""));
  assert(!initregex::regex_match<anchored_abcdef>("^abcdef"));
  assert(!initregex::regex_match<anchored_abcdef>("abcdef$"));
  assert(!initregex::regex_match<anchored_abcdef>("^abcdef$"));

  // Test escaped circumflexes and dollar-signs
  assert(initregex::regex_match<escaped_anchor>("^^$$"));
  assert(!initregex::regex_match<escaped_anchor>(""));
  assert(!initregex::regex_match<escaped_anchor>("^^^$$$"));
}
