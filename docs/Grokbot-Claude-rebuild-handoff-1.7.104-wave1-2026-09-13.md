# Grokbot → Claude handoff — Community Rebuild Factory wave 1 (Skyrim AE 1.7.104)

**From:** Grokbot / Chief of Staff (for Jo Hoeflich)  
**Date:** 2026-09-13 (America/New_York)  
**For:** Claude (build / compile lane)  
**Jo’s PC:** Taquitos  

This file is the single packet Claude needs for the next work. Do **not** ask Jo to re-explain format-5 or dig for passwords.

---

## Mission (what Jo needs from Claude)

**Build three drop-in SKSE DLLs** that load on:

| Piece | Version |
|-------|---------|
| Skyrim AE | **1.7.104.0** |
| SKSE | **2.3.1** |
| Address Library | `versionlib-1-7-104-0.bin` (**format 5**) |

Wave 1 (priority order):

1. **Object Impact Framework** → `ObjectImpactFramework.dll` (Apache-2.0)  
2. **Description Framework** → `DescriptionFramework.dll` (GPL-3.0 — ship rebuild **source** with binary)  
3. **Grab And Throw** → `po3_GrabAndThrow.dll` (MIT)

**Success =** each DLL boots under SKSE with **no** “Unsupported address library format: 5” / hard popup, plus the smoke checks below. Prefer real rebuilds over “leave disabled.”

---

## Factory repo (already live)

| Item | Value |
|------|--------|
| GitHub | https://github.com/vortexenterprisesllc2024-png/skyrim-community-rebuild-factory |
| Account | `vortexenterprisesllc2024-png` (Google SSO: `vortexenterprisesllc2024@gmail.com`) |
| Branch | `main` (scaffold + recipes pushed 2026-09-13) |

**Read in that repo first:**

- `README.md` — project framing  
- `queue/1.7.104.yaml` — queue + licenses  
- `docs/Grokbot-research-1.7.104-format5.md` — root cause writeup  
- `docs/why-not-a-bridge.md` — do **not** invent a universal Address Library bridge  
- `scripts/oif-rebuild.md`  
- `scripts/description-framework-rebuild.md`  
- `scripts/grab-and-throw-rebuild.md`  
- `scripts/README.md`  

Recipes are the step-by-step truth. This handoff is the assignment + constraints.

Local mirror on Taquitos (after push clone):

`C:\Users\hoefl\Downloads\skyrim-work\skyrim-community-rebuild-factory\`

Working trees / outputs Jo expects:

```text
C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src\          # clones
C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\oif\
C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\description-framework\
C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\grab-and-throw\
```

---

## Root cause (one paragraph)

Address Library for 1.7.104 is **correct**. Shipping DLLs were linked to **old CommonLib** that only knows formats 1/2 (VR). Format **5** needs CommonLib with **`Format::SSEv5` / `load_v5`** — current `alandtse/CommonLibVR` branch **`ng`** (CommonLibSSE-NG ≥ ~6.6–6.7; tip was ~8.0.1 / `d13d10a0…` on 2026-09-13 — **re-resolve**). Renaming bins or reinstalling Address Library will not fix this. There is **no safe universal on-the-fly bridge**; product is rebuilds.

---

## Wave-1 targets (exact)

### 1) Object Impact Framework — PRIORITY

| | |
|--|--|
| Nexus | https://www.nexusmods.com/skyrimspecialedition/mods/149484 |
| Upstream | https://github.com/arachnocid/ObjectImpactFramework |
| Base | tag **`1.6.8`** |
| License | Apache-2.0 |
| Failure | format 5 hard popup |
| Fix | Edit `cmake\ports\commonlibsse-ng\portfile.cmake` — bump `REF` to SSEv5-capable `ng` SHA + matching SHA512; `cmake --preset AE` then build |
| Out | `ObjectImpactFramework.dll` + `LICENSE` + `BUILD-NOTES.md` |
| Smoke | No format-5 popup; Activate / Grab / Throw / Hit still work with existing OIF JSON packs |

**CIF (Core Impact Framework) does NOT replace OIF.** Do not skip OIF because CIF is installed.

### 2) Description Framework

| | |
|--|--|
| Nexus | https://www.nexusmods.com/skyrimspecialedition/mods/105799 |
| Upstream | https://github.com/Nightfallstorm/DescriptionFramework |
| Base | **2.1.2** (tag if present) |
| License | **GPL-3.0** — binary redistribution **must** include rebuild source zip |
| Failure | format 5 |
| Fix | Bump submodule `extern/CommonLibSSE-NG` to `alandtse/CommonLibVR` `ng`; preset `descriptionframework`; Release build |
| Out | `DescriptionFramework.dll` + `LICENSE` + source zip + `BUILD-NOTES.md` |
| Smoke | No format-5 popup; `*_DESC` packs still inject inventory/enchant text |

### 3) Grab And Throw (powerof3)

| | |
|--|--|
| Nexus | https://www.nexusmods.com/skyrimspecialedition/mods/120460 |
| Upstream | https://github.com/powerof3/GrabAndThrow |
| Base | tag **`v2.1.2`** |
| License | MIT |
| Failure | `po3_GrabAndThrow.dll` hard popup on Jo boot |
| Fix | Point CommonLib at SSEv5 via `$env:CommonLibSSEPath` → `alandtse/CommonLibVR` `ng` (prefer over fighting powerof3 submodule); `cmake --preset vs2022-windows-vcpkg-ae` → `buildae` Release |
| Out | `po3_GrabAndThrow.dll` + `LICENSE` + `BUILD-NOTES.md` |
| Smoke | No popup; hold **E** grab, **R** throw; keep Jo’s existing `po3_GrabAndThrow.ini` |

OIF `"Throw"` JSON soft-depends on Grab And Throw — rebuild both.

### Explicitly skip

- **Enchanting Cost Formula Fix** (Nexus 62144) — no public source → leave skipped; ESP / Scrambled Bugs fallbacks OK.  
- **Fleikon BTPS** — already catalogued / installed (~3596800-byte community DLL). Do not rebuild unless Jo asks.  
- Universal Address Library “bridge” / binary patcher as the product — **no**.

---

## Toolchain reality (important)

As of 2026-09-13, **Taquitos does not have** Visual Studio / CMake / vcpkg ready for these builds.

Claude should either:

1. **Build elsewhere** (Claude’s environment / another machine with VS 2022 + vcpkg), then drop DLLs into the output dirs above / give Jo a zip; or  
2. **Tell Jo exactly what to install** on Taquitos (VS 2022 C++ workload, CMake, vcpkg path) in a short checklist — Jo executes system installs; agents do not reboot or drive BIOS/drivers.

Do **not** ask Jo for Cursor / GitHub passwords. GitHub Google SSO is fine if Claude needs Jo at the browser.

---

## Deliverables checklist (hand back to Jo / Grokbot)

For **each** of the three mods:

- [ ] DLL in the matching `...\1.7.104\<mod>\` folder (or a zip Jo can unpack there)  
- [ ] `LICENSE` (upstream)  
- [ ] `BUILD-NOTES.md` — upstream tag/commit, CommonLib SHA, cmake commands, DLL size, SHA256, compile host, known caveats  
- [ ] Description Framework only: rebuild **source** zip (GPL)  
- [ ] One-line smoke result: pass / fail / not tested  

Also:

- [ ] Prefer PR or commit into `skyrim-community-rebuild-factory` under `releases/1.7.104/` (or say why not)  
- [ ] If an official author 1.7.104 build appears mid-work, **prefer author DLL** and mark community rebuild superseded  

**Do not** ship JSON/INI replacements — **DLL only** (plus DF source zip). Keep Jo’s packs/settings.

---

## Already done on Jo’s side (do not redo)

- Repo created + scaffold pushed to GitHub  
- Wave-1 recipes written in `scripts/`  
- OIF temporarily disabled so game can boot; restore path = this rebuild  
- Description Framework + Enchanting Cost parked for format 5  
- EngineFixes: FormCaching + TreeLodReferenceCaching set `false` in `EngineFixes.toml` after a cell-entry CTD (setting tweak, not stripping the mod)  
- Fleikon BTPS patch already present  
- RaceMenu / Fuz Ro D’oh soft DLL warnings: expected; Jo hits No; leave installed  

---

## Hard rules for Claude

1. **One live version** of each mod — no install/FOMOD loops in Vortex.  
2. Do not claim work done until Jo can SKSE-boot at least to **main menu with music** with the new DLLs (or you document exact remaining blocker).  
3. Files for Jo/other AIs: keep **`Grokbot`** in filenames when you write new handoff/status files.  
4. System-layer (BIOS, GPU drivers, HAGS, reboot-to-firmware): **out of scope** — escalate to Jo / Claude system lane separately; Skyrim lane is mods/DLLs only.  
5. No public Nexus publish until Jo says yes. GitHub repo already exists; further pushes OK if Jo’s account is available.

---

## Suggested Claude work order

1. Confirm VS2022 + CMake + vcpkg on the build host (or produce Jo install checklist).  
2. Resolve current `alandtse/CommonLibVR` `ng` SHA; verify `SSEv5` in `include/REL/IDDB.h`.  
3. Build **OIF 1.6.8** → package.  
4. Build **Description Framework 2.1.2** → package + GPL source.  
5. Build **Grab And Throw v2.1.2** → package.  
6. Return deliverables + short status Jo can paste to Grokbot.

---

## Contacts / ownership

- Player / PC owner: **Jo Hoeflich** (Taquitos)  
- Orchestration: Grokbot Chief of Staff  
- Skyrim Vortex day-to-day: Skyrim agent (mods only)  
- This rebuild factory seed: Jo + Grokbot  

**End of Grokbot handoff.**
