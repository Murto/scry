#pragma once

/**
 * Custom inline specifier for making a best-effort at ensuring inlining
 */
#if defined(__clang__)
#define SCRY_INLINE [[gnu::always_inline]] inline
#elif defined(__GNUC__) || defined(__GNUG__)
#define SCRY_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
#define SCRY_INLINE __forceinline inline
#endif
