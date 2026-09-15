# BUILD-NOTES - Honed Metal (SKSE plugin) - Skyrim AE 1.7.104.0 / SKSE 2.3.1

**This folder still holds the pre-fix factory DLL** (workflow run 35011091283: `getCachedItemCount` only, `pluginVersion` 7). It is **not** binary-compatible with Honed Metal 1.26.1 scripts. Do not install it. A Windows/MSVC factory rebuild of the `getItemsCount` / `pluginVersion` 8 source will replace these artifacts.

Community rebuild from the author's published source. Not an official author build. Built on a GitHub Actions `windows-2022`
runner by the skyrim-community-rebuild-factory workflow (run 35011091283, 2026-09-15 19:02 UTC).
This replaces the 15 Sep 2026 version-tag hex patch of the 1.6.1170 DLL, which SKSE loaded and which crashed before the main menu.

| Field | Value |
|---|---|
| Upstream | Nexus 61015 (Honed Metal - NPC Crafting and Enchanting Services), author a_retarded_monkey |
| Source used | Nexus 61015 file 466884 `Source Code` v7 - unmodified; per-file SHA-256 below; the 1.26.1 FOMOD binary reports plugin version 8, this source says 7 (the author published no later source) |
| Upstream licence | per the Nexus 61015 page / author (source ships with derivatives - this zip and the repo copy) |
| Build model | classic skse64 plugin: ianpatt/skse64 compiled as a static library + skse64_common + ianpatt/common (author's ReadMe.txt), NOT CommonLibSSE |
| skse64 | https://github.com/ianpatt/skse64 tag `v2.3.1` (commit `7ff865f4a27d6dc936ab5fd0533ff2d706c8f857`): CURRENT_RELEASE_RUNTIME = RUNTIME_VERSION_1_7_104, SKSE 2.3.1 |
| common | https://github.com/ianpatt/common commit `64e233c096735551f6ac9a773726a8a3960e46cd` |
| Configure | `cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=<extern> -DSKSE64_ROOT=<skse64>` (wrapper CMakeLists.txt in the src zip) |
| Build | `cmake --build build --config Release --target HonedMetal` (v143, /MT, C++14, NDEBUG;_CONSOLE as in HonedMetal.vcxproj Release x64) |
| DLL | `HonedMetal.dll` |
| DLL size | 246272 bytes |
| DLL SHA256 | `b8a30f07738b677be70d5f3d84d5b051e3f0cec56b52c5ea2b583b03c99888f9` |
| Version block | compatibleVersions[0] = 1.7.104.0 (0x01070680), no address-library-independence flags - correct for a classic plugin, which must be rebuilt for each game version (verified by verify_version_block.py in the workflow) |
| Compiler | MSVC (Visual Studio 2022, v143) on windows-2022; see the workflow log for the exact `cl.exe` version |
| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to the main menu with music, see `HonedMetal.dll (...) loaded correctly` in skse64.log, then run Farengar's enchant service |

## Source files (Nexus 61015 file 466884, unmodified)

- `e5e231044cc14dc8ae0caf71ed10744da96692a1681f26203e054147f8a43d2f  HonedMetal.vcxproj`
- `9bad0e6758b5a8b170e4d0165f2326731eb98dfab25f2d9ccce157543dbe7cee  HonedMetal.vcxproj.filters`
- `1dacc0ce39b6fffbb7aa53ccf63078be0ec9148472953de5028942f4ae9048eb  ReadMe.txt`
- `7741dbc66698e1db9499befbdab99749e7c4346e19cab5f9e7e9c51486f44432  src\CraftingUtils.cpp`
- `5f348fa433e7c8673bf5fe9ea899579e57e0afdb6c1dde6779bc8760828a6392  src\CraftingUtils.h`
- `59b73ad99d15ce8774f4b04d265f7bfd37ec88961aebde3f2ec2d8cc8f63e5aa  src\IniParser.cpp`
- `aae14121014ecd73a6e1f19be057c42aabdb53d48dac24f89cb9a7d7636319d2  src\IniParser.h`
- `2a5545fb11079c467066ccbfd9ad062cc067d24c9f0986a9c558ab3974d36308  src\main.cpp`
- `99fc266b16fa3c562161d0e46742e07928e3cf61da49a59d31140429387791e8  src\SKSE_resolver.h`

## Source changes made for this rebuild

- None to the author's files. The wrapper CMakeLists.txt reproduces HonedMetal.vcxproj's Release|x64 settings and the ReadMe's static-library recipe; skse64.cpp (the SKSE DLL's own DllMain and loader glue) is left out of the static library, as a plugin must not carry it.
- TARGET_RUNTIME resolves to CURRENT_RELEASE_RUNTIME from the 2.3.1 headers (SKSE_resolver.h), so the runtime check in validate_plugin() and compatibleVersions[0] are both 1.7.104.0 without touching main.cpp.

## Caveats

- Struct layouts (InventoryEntryData, ExtraContainerChanges, DataHandler::arrPERK, EquipManager, MenuManager) come from SKSE 2.3.1's headers for 1.7.104. If SKSE's own headers lag the game for any of these, the symptom is wrong item stats or a crash inside Honed Metal's natives, not a load failure.
- Eight Papyrus natives and one Scaleform inventory callback; the first real exercise is the Farengar (or any Honed Metal NPC) enchant/temper service. Test with a save you can throw away.
- Keep HonedMetal.esp enabled; replace only HonedMetal.dll (Data and the Honed Metal 1.26 Vortex staging copy). Do not FOMOD-reinstall.
- If the author publishes a 1.7.104 build, it supersedes this one.
