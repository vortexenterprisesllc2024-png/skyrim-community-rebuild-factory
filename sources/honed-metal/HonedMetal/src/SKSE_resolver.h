#ifndef SKSE_RESOLVER_H
# define SKSE_RESOLVER_H

# define MAKE_STRING(s) #s
# define EXPAND_N_STRING(e) MAKE_STRING(e)
# define CONCAT(a, b) EXPAND_N_STRING(a ## b)
# define RESOLVE_PATH(prefix, path) CONCAT(prefix, path)

# if defined _WIN64
#  define VERSION_PATH skse64_common/
#  define SKSE_FOLDER skse64/
# elif defined _WIN32
#  define VERSION_PATH skse
#  define SKSE_FOLDER skse
# else
#  error "Uknown environment."
# endif
# include RESOLVE_PATH(VERSION_PATH, skse_version.h)

# if defined RUNTIME_VERSION_1_9_32_0
#  define LEGENDARY_EDITION 1
#  define GET_PERK_ENTRIES(gamedata) (*gamedata).perks.arr.entries
#  define GET_PERKS_COUNT(gamedata) (*gamedata).perks.count
#  define TARGET_RUNTIME RUNTIME_VERSION_1_9_32_0
# elif defined MAKE_EXE_VERSION
#  define TARGET_RUNTIME CURRENT_RELEASE_RUNTIME
#  define GET_PERK_ENTRIES(gamedata) (BGSPerk**)(*gamedata).arrPERK.entries
#  define GET_PERKS_COUNT(gamedata) (*gamedata).arrPERK.count
#  if PACKED_SKSE_VERSION >= MAKE_EXE_VERSION(2, 1, 0)
#   define CASH_GRAB_EDITION 1
#  endif
# endif
// Additional dependencies needed by SKSE headers
# include <string>
# include <Windows.h>
# include "common/ITypes.h"
# include "common/IErrors.h"

#endif //SKSE_resovler.h
