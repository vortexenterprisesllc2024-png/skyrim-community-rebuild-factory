# Wave 1 rebuild recipes — Windows builder guide

**Runtime target:** Skyrim AE `1.7.104.0` · SKSE `2.3.1` · Address Library `versionlib-1-7-104-0.bin` (**format 5 / SSEv5**)

**These are recipes only.** Completing the steps below produces rebuild *candidates*. Do **not** mark queue items `built` / `shipped` until smoke tests pass and artifacts are copied into the output tree.

**Output root (Jo’s machine):**

```text
C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\
```

Per-mod folders under that root:

| Mod | Folder | DLL |
|-----|--------|-----|
| Object Impact Framework | `oif\` | `ObjectImpactFramework.dll` |
| Description Framework | `description-framework\` | `DescriptionFramework.dll` |
| Grab And Throw | `grab-and-throw\` | `po3_GrabAndThrow.dll` |
| Auto Plant Everything | `autoplanteverything\` | `AutoPlantEverything.dll` |

---

## Prerequisites (one-time)

1. **Visual Studio 2022** — Desktop development with C++ (MSVC v143).
2. **CMake** ≥ 3.21 on `PATH` (`cmake --version`).
3. **Git** on `PATH`.
4. **vcpkg**
   - Clone / bootstrap if needed.
   - Set user env:
     - `VCPKG_ROOT` = path to vcpkg root (required by OIF + GrabAndThrow + DF CMakeLists).
     - Also set `VCPKG_INSTALLATION_ROOT` to the same path (Description Framework’s `CMakePresets.json` reads this name).
5. **Game smoke stack (already on Jo’s install):**
   - Skyrim AE 1.7.104.0
   - SKSE 2.3.1
   - Address Library with `Data\SKSE\Plugins\versionlib-1-7-104-0.bin` — **do not replace** with an older format bin.
6. Optional but useful: open **x64 Native Tools Command Prompt for VS 2022** for all cmake/build commands.

### CommonLib rule (all three mods)

Failing DLLs were linked against CommonLib that only understands Address Library formats **1 / 2**. Fix = rebuild against **`alandtse/CommonLibVR` branch `ng`** (or equivalent CommonLibSSE-NG **≥ ~6.6–6.7 / current 8.x**) that includes:

- `REL::IDDB::Format::SSEv5`
- `load_v5(...)` / `header_v5_t`

As of 2026-09-13, tip of `ng` was approximately:

```text
d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd   # CommonLibSSE-NG 8.0.1 — verify before pin
```

**Always re-check** before building:

```powershell
git ls-remote https://github.com/alandtse/CommonLibVR.git refs/heads/ng
```

Confirm SSEv5 is present in that commit:

```powershell
# after cloning CommonLibVR @ ng tip
Select-String -Path include\REL\IDDB.h -Pattern "SSEv5"
```

---

## Wave 1 order

1. [oif-rebuild.md](./oif-rebuild.md) — **PRIORITY** (Apache-2.0). CIF does **not** replace OIF.
2. [description-framework-rebuild.md](./description-framework-rebuild.md) — GPL-3.0 (keep rebuild source with the binary).
3. [grab-and-throw-rebuild.md](./grab-and-throw-rebuild.md) — MIT, tag **v2.1.2**.
4. [autoplanteverything-rebuild.md](./autoplanteverything-rebuild.md) — original plugin 0.2.0. Does not replace `Grokbot-AllPlantablePlants.esp`.

Catalog-only (no rebuild this wave): Fleikon BTPS — already on Jo’s list (`queue/1.7.104.yaml` → `catalog_existing`).

Out of scope: Enchanting Cost Formula Fix (`skip_no_source`).

---

## Shared deliverable checklist (every mod)

After a successful Release build:

1. Create the output folder under `...\1.7.104\<mod-id>\`.
2. Copy **only the new DLL** (keep the player’s existing JSON / INI / `*_DESC` packs from the original Nexus install).
3. Copy upstream `LICENSE` into the same folder.
4. Write `BUILD-NOTES.md` in that folder with:
   - Upstream URL + tag/commit
   - Exact CommonLib commit used
   - CMake configure + build commands
   - Compiler (VS 2022)
   - DLL file size (bytes)
   - **SHA256** of the DLL
   - Smoke-test result (pass/fail + log paths)
   - Any remaining RelocationID / hook failures
5. Record SHA256:

```powershell
Get-FileHash -Algorithm SHA256 path\to\Your.dll | Format-List
```

6. Update `queue/1.7.104.yaml` status only after smoke passes (`built_pending_pack` or similar) — recipes alone leave status at **`rebuilding`**.

---

## Shared smoke protocol

1. Disable / park the **old** DLL (or overwrite after backup).
2. Drop the new DLL into `Data\SKSE\Plugins\` (via Vortex/MO2 staging as Jo normally installs).
3. Launch via **skse64_loader.exe**.
4. **Must not** see:
   - `Unsupported address library format: 5`
   - REL “failed to open address library”
5. Check logs under:

```text
Documents\My Games\Skyrim Special Edition\SKSE\
```

6. Run the mod-specific feature checks in each recipe.
7. If the DLL loads but crashes on use, **do not ship** — log the faulting address / SKSE plugin log and stop.

---

## CIF vs OIF (do not skip OIF)

**Core Impact Framework (CIF)** = actor/combat impacts.  
**Object Impact Framework (OIF)** = world-object Activate/Grab/Throw/Hit JSON packs.

CIF does **not** restore OIF-dependent content. Rebuild OIF even if CIF is present or loads cleanly.

---

## Prefer official author builds

If arachnocid / Nightfallstorm / powerof3 publish an official 1.7.104 (or format-5-capable) DLL, prefer that and mark the community rebuild `superseded`.
