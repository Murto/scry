#pragma once

#include <cstdint>

namespace scry {

using trait_type = uint16_t;

namespace trait {

/**
 * Traits used to select the regex grammar used, and other options.
 *
 * Note: Optimization is always performed because there is no runtime drawback,
 *       so `optimize` has no effect. Locale is not determined at compile-time
 *       so `collate` has no effect.
 */
constexpr static const trait_type icase = 1;
constexpr static const trait_type nosubs = 2;
constexpr static const trait_type optimize = 4;
constexpr static const trait_type collate = 8;
constexpr static const trait_type multiline = 16;
constexpr static const trait_type ECMAScript = 32;
constexpr static const trait_type basic = 64;
constexpr static const trait_type extended = 128;
constexpr static const trait_type awk = 256;
constexpr static const trait_type grep = 512;
constexpr static const trait_type egrep = 1024;

} // namespace trait

} // namespace scry
