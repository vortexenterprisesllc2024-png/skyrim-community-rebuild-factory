#pragma once

// NOMINMAX before the Windows headers. Jo's rebuild factory had to add this when
// building against CommonLibSSE-NG 8.x (ci/build-oif.ps1) - without it the Windows
// min/max macros eat std::numeric_limits<>::max() inside NG's own headers.
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

using namespace std::literals;   // for the "..."sv literal in main.cpp
