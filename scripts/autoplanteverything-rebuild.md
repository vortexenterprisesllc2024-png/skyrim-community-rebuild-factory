# Build recipe — Auto Plant Everything 0.2.0

| Field | Value |
|-------|--------|
| Queue id | `autoplanteverything` |
| Upstream | this repo, `sources/autoplanteverything/` (original plugin, not a third-party fork) |
| Author | Jo Hoeflich |
| License | plugin source MIT; DLL is GPL-3.0-or-later because it links CommonLibSSE-NG |
| DLL out | `AutoPlantEverything.dll` |
| Player zip | `AutoPlantEverything-0.2.0.zip` (`SKSE/Plugins/AutoPlantEverything.dll` + `AutoPlantEverything.ini`) |
| CommonLib | `alandtse/CommonLibVR` `d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd` (`Format::SSEv5`) |

`Grokbot-AllPlantablePlants.esp` is Jo's live planter fix. This recipe does not replace it.

## What CI runs

`ci/build-autoplanteverything.ps1`, same CommonLib overlay treatment as `ci/build-oif.ps1`:

1. Copy `sources/autoplanteverything` to the runner temp tree.
2. Rewrite `cmake/ports/commonlibsse-ng` REF/SHA512 to the SSEv5 CommonLib commit (NG 8.0.1 port fixes: current vcpkg cmake helpers, MinHook for patch safety, `COPYING.txt`).
3. `cmake --preset AE` / `cmake --build --preset AE` (VS 2022, triplet `x64-windows-static-md`).
4. Fail if `AutoPlantEverything.dll` is missing or under 50 KB, or if the version block is not address-library + no-structs.
5. Zip only the DLL and the INI. No `ModuleConfig.xml`. No ESL.

`tools/generate_esl.py` still refuses to write a FormList override unless it read a real Data folder and is adding pairs. CI re-runs that refusal. Do not remove those checks, and do not pass `--allow-empty`.
