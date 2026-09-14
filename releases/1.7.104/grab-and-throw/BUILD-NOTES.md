# BUILD-NOTES - Grab And Throw - Skyrim AE 1.7.104.0 / SKSE 2.3.1 / Address Library format 5

Community rebuild. Not an official author build. Built on a GitHub Actions `windows-latest` runner by the
skyrim-community-rebuild-factory workflow (run 34795479373, 2026-09-14 01:34 UTC).

| Field | Value |
|---|---|
| Upstream | https://github.com/powerof3/GrabAndThrow tag `v2.1.2` (commit `7f5445c9b7859072a369a4310cfcaa9305fb3bc4`) |
| Upstream licence | MIT (LICENSE alongside) |
| Nexus | https://www.nexusmods.com/skyrimspecialedition/mods/120460 |
| CommonLib | alandtse/CommonLibVR branch ng, commit `d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd` (CommonLibSSE-NG 8.0.1) via CommonLibSSEPath (upstream submodule powerof3/CommonLibSSE not used) |
| vcpkg | microsoft/vcpkg `a1cae005c39be7b18ba319fced856b68d7276271`, triplet x64-windows-static |
| Configure | ``cmake --preset vs2022-windows-vcpkg-ae -DCOPY_BUILD=OFF -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON`` (Visual Studio 17 2022, v143, /MT, SKYRIM_AE) |
| Build | ``cmake --build buildae --config Release`` |
| DLL | `po3_GrabAndThrow.dll` |
| DLL size | 966144 bytes |
| DLL SHA256 | `84ac3ee10775483b3f5a7c258cb38e040eb3515b446592403b62035737cef6c6` |
| Compiler | MSVC (Visual Studio 2022, v143) on windows-latest; see workflow log for exact `cl.exe` version |
| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to main menu + in-game check before this counts |

## Source changes made for this rebuild

- src/Hooks.cpp: PlayerCharacter::grabType is a direct member in powerof3/CommonLibSSE but lives in the versioned runtime-data block in CommonLibSSE-NG; read it through GetPlayerRuntimeData() (two call sites, same field, same GrabbingType compare).
- src/GrabThrowHandler.cpp: Actor::currentProcess and PlayerCharacter::grabSpring likewise live in NG runtime-data blocks (grabSpring inside GrabData); read through GetActorRuntimeData() / GetPlayerRuntimeData().grabData (three call sites, same fields, same offsets).
- src/PCH.h: NG 8.x no longer defines the SKSEAPI calling-convention macro; defined as __cdecl (a no-op on x64) so the SKSEPlugin_Load/Query signatures compile unchanged.
- vcpkg.json: builtin-baseline 14bb451131ccf6be50a63a8d9dfe7980e46b5958 -> a1cae005c39be7b18ba319fced856b68d7276271; added directxtk and rapidcsv, which CommonLibSSE-NG's CMakeLists requires (powerof3's fork did not). Existing deps (clib-util, rsm-binary-io, spdlog, xbyak) untouched.
- CommonLib: extern/CommonLibSSE submodule (powerof3/CommonLibSSE 7312db81, no format 5) left uninitialised; CommonLibSSEPath=D:\a\_temp\src\CommonLibVR -> alandtse/CommonLibVR d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd (NG 8.0.1, Format::SSEv5). Upstream CMakeLists already resolves CommonLib from that env var when extern/ is empty.

## Licence note

CommonLibSSE-NG relicensed to GPL-3.0-or-later (with the Modding Exception) on 2026-07-25, before format-5
support was added. Every DLL linked against it is a GPL-3.0-or-later combined work, so the matching source
(this plugin's tree as patched + the CommonLib commit above) ships next to the binary as `*-rebuild-src.zip`.
The plugin's own code keeps its upstream licence (see LICENSE in this folder).

## Caveats

- Three call-site hooks use fixed offsets inside Address-Library-resolved functions (44005+0x50, 38185+0x200, 25850+0xF4) from the 1.6.x era. If boot is clean but grabbing/throwing crashes, that is hook drift on 1.7.104 - report the faulting address.
- Built against CommonLibSSE-NG instead of powerof3 own CommonLibSSE fork: struct layouts for AE 1.7.x come from NG 8.0.1 (which has explicit 1.7.99+ layout work). Same engine functions, different RE library - test hold-E grab, R throw, and an OIF "Throw" rule.
- Replace only po3_GrabAndThrow.dll; keep the existing po3_GrabAndThrow.ini.
- Prefer an official powerof3 1.7.104 build if one appears; then this rebuild is superseded.

