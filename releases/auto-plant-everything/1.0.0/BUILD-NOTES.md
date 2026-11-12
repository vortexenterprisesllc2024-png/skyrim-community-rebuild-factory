# BUILD-NOTES - Auto Plant Everything - Skyrim AE 1.7.104.0 / SKSE 2.3.1 / Address Library format 5

Community rebuild. Not an official author build. Built on a GitHub Actions `windows-latest` runner by the
skyrim-community-rebuild-factory workflow (run 43123324704, 2026-11-12 00:10 UTC).

| Field | Value |
|---|---|
| Source | sources/auto-plant-everything (Jo Hoeflich, original work) |
| CommonLib | alandtse/CommonLibVR branch ng, commit `d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd` (CommonLibSSE-NG 8.0.1) via OIF 1.6.8's overlay port, patched as in ci/build-oif.ps1 |
| vcpkg | microsoft/vcpkg `b58529e33332fbb6dfe20317a82009b78ceeab36`, triplet x64-windows-static-md (release only) |
| Build | Visual Studio 17 2022, x64, Release, /MD |
| DLL | `AutoPlantEverything.dll` |
| DLL size | 622080 bytes |
| DLL SHA256 | `75ac8b61c1f1a66941b3c3727520bd95627124552c86cf695ec727bb647ecd02` |
| Compiler | MSVC (Visual Studio 2022, v143) on windows-latest; see workflow log for exact `cl.exe` version |
| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to main menu + in-game check before this counts |

## Licence note

CommonLibSSE-NG relicensed to GPL-3.0-or-later (with the Modding Exception) on 2026-07-25, before format-5
support was added. Every DLL linked against it is a GPL-3.0-or-later combined work, so the matching source
(this plugin's tree as patched + the CommonLib commit above) ships next to the binary as `*-rebuild-src.zip`.
The plugin's own code keeps its upstream licence (see LICENSE in this folder).

## Caveats

- Own plugin, not a rebuild of someone else's mod.

