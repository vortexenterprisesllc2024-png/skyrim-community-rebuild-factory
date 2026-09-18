#pragma once

// Plugin / zip / DLL product version. 4.0 is a new line, not Experience 3.x.
#define ADVENTUREXP_VERSION_MAJOR 4
#define ADVENTUREXP_VERSION_MINOR 0
#define ADVENTUREXP_VERSION_PATCH 0
#define ADVENTUREXP_VERSION_STRING "4.0.0"

namespace AdventureXP
{
    inline constexpr const char* kVersionString = ADVENTUREXP_VERSION_STRING;
    inline constexpr int kVersionMajor = ADVENTUREXP_VERSION_MAJOR;
    inline constexpr int kVersionMinor = ADVENTUREXP_VERSION_MINOR;
    inline constexpr int kVersionPatch = ADVENTUREXP_VERSION_PATCH;
}
