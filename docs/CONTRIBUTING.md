# Contributing a rebuild

## Prerequisites
- Visual Studio 2022 (C++ desktop)
- CMake, vcpkg
- Skyrim AE 1.7.104 + SKSE 2.3.1 + current Address Library (format 5) for smoke tests

## Recipe (typical CommonLibSSE-NG / alandtse pin)
1. Clone upstream at the last good tag.
2. Point CommonLib / `commonlibsse-ng` port REF to current `alandtse/CommonLibVR` `ng` (must include `Format::SSEv5` / `load_v5`).
3. Build AE preset Release.
4. Replace only the DLL (keep JSON/INI/scripts from the player’s original mod).
5. Smoke test: SKSE load → no format-5 popup → feature check (see queue entry).
6. Open PR with: upstream URL + license, CommonLib commit, DLL hash/size, smoke notes, known risks.

## Rules
- Prefer author release when it appears; mark our build `superseded`.
- No closed-source reverse engineering in this repo without explicit legal review.
- GPL rebuilds: source of the rebuild must stay available (link PR / release zip).
