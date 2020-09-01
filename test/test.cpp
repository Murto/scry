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
constexpr static const char some_abcdef_pattern[] = R"([abcdef]*)";
constexpr static const char some_lower_pattern[] = R"([abcd-wxyz]*)";
constexpr static const char not_some_lower_pattern[] = R"([^abcd-wxyz]*)";
constexpr static const char upper_cc_pattern[] = R"([[:upper:]]*)";
constexpr static const char lower_cc_pattern[] = R"([[:lower:]]*)";
constexpr static const char alpha_cc_pattern[] = R"([[:alpha:]]*)";
constexpr static const char digit_cc_pattern[] = R"([[:digit:]]*)";
constexpr static const char xdigit_cc_pattern[] = R"([[:xdigit:]]*)";
constexpr static const char alnum_cc_pattern[] = R"([[:alnum:]]*)";
constexpr static const char punct_cc_pattern[] = R"([[:punct:]]*)";
constexpr static const char blank_cc_pattern[] = R"([[:blank:]]*)";
constexpr static const char space_cc_pattern[] = R"([[:space:]]*)";
constexpr static const char cntrl_cc_pattern[] = R"([[:cntrl:]]*)";
constexpr static const char graph_cc_pattern[] = R"([[:graph:]]*)";
constexpr static const char print_cc_pattern[] = R"([[:print:]]*)";
constexpr static const char word_cc_pattern[] = R"([[:word:]]*)";

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
  using some_abcdef = scry::regex<some_abcdef_pattern>;
  using some_lower = scry::regex<some_lower_pattern>;
  using not_some_lower = scry::regex<not_some_lower_pattern>;
  using upper_cc = scry::regex<upper_cc_pattern>;
  using lower_cc = scry::regex<lower_cc_pattern>;
  using alpha_cc = scry::regex<alpha_cc_pattern>;
  using digit_cc = scry::regex<digit_cc_pattern>;
  using xdigit_cc = scry::regex<xdigit_cc_pattern>;
  using alnum_cc = scry::regex<alnum_cc_pattern>;
  using punct_cc = scry::regex<punct_cc_pattern>;
  using blank_cc = scry::regex<blank_cc_pattern>;
  using space_cc = scry::regex<space_cc_pattern>;
  using cntrl_cc = scry::regex<cntrl_cc_pattern>;
  using graph_cc = scry::regex<graph_cc_pattern>;
  using print_cc = scry::regex<print_cc_pattern>;
  using word_cc = scry::regex<word_cc_pattern>;

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

  // Test basic bracket expressions
  assert(scry::regex_match<some_abcdef>(""));
  assert(scry::regex_match<some_abcdef>("a"));
  assert(scry::regex_match<some_abcdef>("b"));
  assert(scry::regex_match<some_abcdef>("c"));
  assert(scry::regex_match<some_abcdef>("d"));
  assert(scry::regex_match<some_abcdef>("e"));
  assert(scry::regex_match<some_abcdef>("f"));
  assert(scry::regex_match<some_abcdef>("abcdef"));
  assert(scry::regex_match<some_abcdef>("fedcba"));
  assert(!scry::regex_match<some_abcdef>("g"));

  // Test range expressions
  assert(scry::regex_match<some_lower>(""));
  assert(scry::regex_match<some_lower>("abcdefghijklmnopqrstuvwxyz"));
  assert(!scry::regex_match<some_lower>("A"));
  assert(!scry::regex_match<some_lower>("M"));
  assert(!scry::regex_match<some_lower>("Z"));

  // Test non-matching bracket expressions
  assert(scry::regex_match<not_some_lower>(""));
  assert(scry::regex_match<not_some_lower>("`"));
  assert(scry::regex_match<not_some_lower>("{"));
  assert(scry::regex_match<not_some_lower>("A"));
  assert(scry::regex_match<not_some_lower>("M"));
  assert(scry::regex_match<not_some_lower>("Z"));
  assert(!scry::regex_match<not_some_lower>("abcdefghijklmnopqrstuvwxyz"));
  assert(!scry::regex_match<not_some_lower>("a"));
  assert(!scry::regex_match<not_some_lower>("m"));
  assert(!scry::regex_match<not_some_lower>("z"));

  // Test [:upper:] character class
  assert(scry::regex_match<upper_cc>("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  assert(scry::regex_match<upper_cc>(""));
  assert(!scry::regex_match<upper_cc>("@"));
  assert(!scry::regex_match<upper_cc>("["));
  assert(!scry::regex_match<upper_cc>("a"));
  assert(!scry::regex_match<upper_cc>("z"));

  // Test [:lower:] character class
  assert(scry::regex_match<lower_cc>("abcdefghijklmnopqrstuvwxyz"));
  assert(scry::regex_match<lower_cc>(""));
  assert(!scry::regex_match<lower_cc>("`"));
  assert(!scry::regex_match<lower_cc>("{"));
  assert(!scry::regex_match<lower_cc>("A"));
  assert(!scry::regex_match<lower_cc>("Z"));

  // Test [:alpha:] character class
  assert(scry::regex_match<alpha_cc>(
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  assert(scry::regex_match<alpha_cc>(""));
  assert(!scry::regex_match<alpha_cc>("@"));
  assert(!scry::regex_match<alpha_cc>("["));
  assert(!scry::regex_match<alpha_cc>("`"));
  assert(!scry::regex_match<alpha_cc>("{"));

  // Test [:digit:] character class
  assert(scry::regex_match<digit_cc>("0123456789"));
  assert(scry::regex_match<digit_cc>(""));
  assert(!scry::regex_match<digit_cc>("/"));
  assert(!scry::regex_match<digit_cc>(":"));

  // Test [:xdigit:] character class
  assert(scry::regex_match<xdigit_cc>("0123456789abcdefABCDEF"));
  assert(scry::regex_match<xdigit_cc>(""));
  assert(!scry::regex_match<xdigit_cc>("/"));
  assert(!scry::regex_match<xdigit_cc>(":"));
  assert(!scry::regex_match<xdigit_cc>("`"));
  assert(!scry::regex_match<xdigit_cc>("g"));
  assert(!scry::regex_match<xdigit_cc>("@"));
  assert(!scry::regex_match<xdigit_cc>("G"));

  // Test [:alnum:] character class
  assert(scry::regex_match<alnum_cc>(
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
  assert(scry::regex_match<alnum_cc>(""));
  assert(!scry::regex_match<alnum_cc>("@"));
  assert(!scry::regex_match<alnum_cc>("["));
  assert(!scry::regex_match<alnum_cc>("`"));
  assert(!scry::regex_match<alnum_cc>("{"));
  assert(!scry::regex_match<alnum_cc>("/"));
  assert(!scry::regex_match<alnum_cc>(":"));

  // Test [:punct:] character class
  assert(scry::regex_match<punct_cc>("'()*+,-./:;<=>?@[\\]^_`{|}~"));
  assert(scry::regex_match<punct_cc>(""));
  assert(!scry::regex_match<punct_cc>("0"));
  assert(!scry::regex_match<punct_cc>("9"));
  assert(!scry::regex_match<punct_cc>("a"));
  assert(!scry::regex_match<punct_cc>("z"));
  assert(!scry::regex_match<punct_cc>("A"));
  assert(!scry::regex_match<punct_cc>("Z"));

  // Test [:blank:] character class
  assert(scry::regex_match<blank_cc>(" \t"));
  assert(scry::regex_match<cntrl_cc>(""));
  assert(!scry::regex_match<blank_cc>("m"));
  assert(!scry::regex_match<blank_cc>("M"));
  assert(!scry::regex_match<blank_cc>("~"));

  // Test [:space:] character class
  assert(scry::regex_match<space_cc>(" \t\n\r\f\v"));
  assert(scry::regex_match<space_cc>(""));
  assert(!scry::regex_match<space_cc>("m"));
  assert(!scry::regex_match<space_cc>("M"));
  assert(!scry::regex_match<space_cc>("~"));

  // Test [:cntrl:] character class
  // TODO
  assert(scry::regex_match<cntrl_cc>(""));
  assert(!scry::regex_match<cntrl_cc>("m"));
  assert(!scry::regex_match<cntrl_cc>("M"));
  assert(!scry::regex_match<cntrl_cc>("~"));

  // Test [:graph:] character class
  assert(scry::regex_match<graph_cc>("azAZ09!?~#"));
  assert(scry::regex_match<graph_cc>(""));
  assert(!scry::regex_match<graph_cc>(" "));
  assert(!scry::regex_match<graph_cc>("\t"));
  assert(!scry::regex_match<graph_cc>("\f"));

  // Test [:print:] character class
  assert(scry::regex_match<print_cc>("azAZ09!?~# "));
  assert(scry::regex_match<print_cc>(""));
  assert(!scry::regex_match<print_cc>("\f"));
  assert(!scry::regex_match<print_cc>("\v"));

  // Test [:word:] character class
  assert(scry::regex_match<word_cc>("azAZ09_"));
  assert(scry::regex_match<word_cc>(""));
  assert(!scry::regex_match<word_cc>(" "));
  assert(!scry::regex_match<word_cc>("\t"));
  assert(!scry::regex_match<word_cc>("\f"));
  assert(!scry::regex_match<word_cc>("\v"));
}
