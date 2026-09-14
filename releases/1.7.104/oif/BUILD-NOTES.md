# BUILD-NOTES - Object Impact Framework - Skyrim AE 1.7.104.0 / SKSE 2.3.1 / Address Library format 5

Community rebuild. Not an official author build. Built on a GitHub Actions `windows-latest` runner by the
skyrim-community-rebuild-factory workflow (run 34795479373, 2026-09-14 01:37 UTC).

| Field | Value |
|---|---|
| Upstream | https://github.com/arachnocid/ObjectImpactFramework tag `1.6.8` (commit `ec2b126b5fb0ae98311ceb30fc8acdf28d8936a2`) |
| Upstream licence | Apache-2.0 (LICENSE alongside) |
| Nexus | https://www.nexusmods.com/skyrimspecialedition/mods/149484 |
| CommonLib | alandtse/CommonLibVR branch ng, commit `d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd` (CommonLibSSE-NG 8.0.1) via the upstream vcpkg overlay port (REF/SHA512 bumped) |
| vcpkg | microsoft/vcpkg `a1cae005c39be7b18ba319fced856b68d7276271`, triplet x64-windows-static-md |
| Configure | ``cmake --preset AE`` (Visual Studio 17 2022, x64, /MD, ENABLE_SKYRIM_AE) |
| Build | ``cmake --build --preset AE`` (Release) |
| DLL | `ObjectImpactFramework.dll` |
| DLL size | 2153472 bytes |
| DLL SHA256 | `c4ab8b976acec3dea6ec0688c9a4956c52477ad278e12329954fad4423cd9c45` |
| Compiler | MSVC (Visual Studio 2022, v143) on windows-latest; see workflow log for exact `cl.exe` version |
| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to main menu + in-game check before this counts |

## Source changes made for this rebuild

- include/PCH.h: SKSE::PluginVersionData::HasNoStructUse() was renamed UsesNoStructs() in CommonLibSSE-NG (same flag, kVersionIndependentEx_NoStructUse).
- include/PCH.h: SKSE::Init(a_skse) now installs its own spdlog logger by default; pass { .log = false } so the plugin keeps its own ObjectImpactFramework.log setup (same file name, original pattern).
- include/PCH.h: PluginVersionData::UsesAddressLibrary() takes no argument in NG 8.x (same flag set).
- include/PCH.h: NG 8.x no longer defines the SKSEAPI calling-convention macro; defined as __cdecl (a no-op on x64) so the SKSEPlugin_Load/Query signatures compile unchanged.
- include/PCH.h: NOMINMAX before the Windows headers so std::numeric_limits<>::max() in NG 8.x / dependency headers is not eaten by the min/max macros (OIF code already parenthesises its own uses).
- src/Effects.cpp: RE::DebugNotification() no longer exists in NG 8.x; RE::SendHUDMessage::ShowHUDMessage() is the same engine call (same three arguments).
- src/EventSinks.cpp: MagicSystem::Delivery::kTotal (value 5) is spelled kNone (value 5) in NG 8.x - three switch cases, same value.
- src/Effects.cpp: two COL_LAYER enumerators renamed in NG 8.x (kClutterLarge=29 -> written as the literal 29; kItemPicker=40 -> kItemPick); the raycast layer list keeps the same numeric layers.
- src/Effects.cpp: Actor::GetCollisionFilterInfo() and hkpWorldRayCastInput::filterInfo now use the RE::CFilter wrapper (a struct around the same uint32) instead of a raw uint32_t; same engine call (ID 36559/37560), same bits.
- src/Effects.cpp: BSAudioManager::BuildSoundDataFromDescriptor() was renamed GetSoundHandle() in NG 8.x - same engine function (ID 66404/67666), same arguments.

## Licence note

CommonLibSSE-NG relicensed to GPL-3.0-or-later (with the Modding Exception) on 2026-07-25, before format-5
support was added. Every DLL linked against it is a GPL-3.0-or-later combined work, so the matching source
(this plugin's tree as patched + the CommonLib commit above) ships next to the binary as `*-rebuild-src.zip`.
The plugin's own code keeps its upstream licence (see LICENSE in this folder).

## Caveats

- Hook offsets inside functions (REL::VariantOffset values) are unchanged from the 1.6.x-era source; the Address Library maps function starts, not the call sites inside them. If an OIF event crashes on 1.7.104 while boot is clean, that is hook drift, not a format-5 problem - report the faulting address.
- CIF (Core Impact Framework) is unrelated; keep it installed. Replace only ObjectImpactFramework.dll; keep the existing Data\SKSE\Plugins\ObjectImpactFramework\*.json packs.
- Prefer an official arachnocid 1.7.104 build if one appears; then this rebuild is superseded.

