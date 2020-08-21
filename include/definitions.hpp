#pragma once

// Define custom inline specifier -- works with clang and gcc for now
#define SCRY inline [[gnu::always_inline]]
