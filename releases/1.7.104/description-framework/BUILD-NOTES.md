# BUILD-NOTES - Description Framework - Skyrim AE 1.7.104.0 / SKSE 2.3.1 / Address Library format 5

Community rebuild. Not an official author build. Built on a GitHub Actions `windows-latest` runner by the
skyrim-community-rebuild-factory workflow (run 34795479373, 2026-09-14 01:34 UTC).

| Field | Value |
|---|---|
| Upstream | https://github.com/Nightfallstorm/DescriptionFramework tag `2.1.2` (commit `68ddde353bf119f5510ad85b7b2344cd6c1ccb24`) |
| Upstream licence | GPL-3.0-or-later (LICENSE alongside; corresponding source in the *-rebuild-src.zip) |
| Nexus | https://www.nexusmods.com/skyrimspecialedition/mods/105799 |
| CommonLib | alandtse/CommonLibVR branch ng, commit `d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd` (CommonLibSSE-NG 8.0.1) as submodule extern/CommonLibSSE-NG |
| vcpkg | microsoft/vcpkg `a1cae005c39be7b18ba319fced856b68d7276271`, triplet x64-windows-static |
| Configure | ``cmake --preset descriptionframework -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON`` (Visual Studio 17 2022, v143, /MT) |
| Build | ``cmake --build build --config Release`` |
| DLL | `DescriptionFramework.dll` |
| DLL size | 1091584 bytes |
| DLL SHA256 | `f938535c04427be6b4c56fdd6a4e8da3d4aaf2637ac0bbb69cf3948b39dba963` |
| Compiler | MSVC (Visual Studio 2022, v143) on windows-latest; see workflow log for exact `cl.exe` version |
| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to main menu + in-game check before this counts |

## Source changes made for this rebuild

- src/main.cpp: SKSE::Init(a_skse) in CommonLibSSE-NG 8.x installs its own spdlog logger by default, which would replace the logger InitializeLog() already set up (and drop the bDebug level); { .log = false } keeps the original DescriptionFramework.log behaviour.
- src/main.cpp: PluginVersionData::UsesAddressLibrary() / UsesNoStructs() take no argument in NG 8.x (same flags set).
- src/PCH.h: NG 8.x no longer defines the SKSEAPI calling-convention macro; defined as __cdecl (a no-op on x64) so the SKSEPlugin_Query signature compiles unchanged.
- src/PCH.h: NOMINMAX before the Windows headers - with NG 8.x the min/max macros leak into SimpleIni.h (std::numeric_limits<>::max()) and break the build.
- src/PCH.h + src/Settings.h: fmt 12 (via spdlog 1.16 / NG 8.x) refuses to format enums without a format_as() hook; added format_as() for RE::GFxValue::ValueType and Settings::SkyrimFont so the existing logger lines print the same integer values as before.
- vcpkg.json: builtin-baseline moved from 417119555f155f6044dec7a379cd25466e339873 (2023) to a1cae005c39be7b18ba319fced856b68d7276271 and the fmt 8.0.1 override removed, because NG 8.0.1 requires fmt >= 12.1.0 / spdlog >= 1.16.0. No dependency added or removed.
- extern/CommonLibSSE-NG: submodule 738afc457fdeba050ce328f15225aa3cde674187 (NG 3.6.0, formats 1/2 only) -> d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd (NG 8.0.1, Format::SSEv5).

## Licence note

CommonLibSSE-NG relicensed to GPL-3.0-or-later (with the Modding Exception) on 2026-07-25, before format-5
support was added. Every DLL linked against it is a GPL-3.0-or-later combined work, so the matching source
(this plugin's tree as patched + the CommonLib commit above) ships next to the binary as `*-rebuild-src.zip`.
The plugin's own code keeps its upstream licence (see LICENSE in this folder).

## Caveats

- The six ItemCard hooks are call-site hooks at fixed offsets inside functions resolved by Address Library ID (e.g. RELOCATION_ID(50674, 51569) + 0x7A). Those offsets date from 1.6.x. If the game boots clean but item cards show no description text, or crash on opening a menu, that is hook drift on 1.7.104 - not a format-5 problem.
- Upstream logs a critical warning if po3_Tweaks is not loaded; unchanged here.
- Replace only DescriptionFramework.dll; keep the existing *_DESC.ini description packs.
- Prefer an official Nightfallstorm 1.7.104 build if one appears; then this rebuild is superseded.

