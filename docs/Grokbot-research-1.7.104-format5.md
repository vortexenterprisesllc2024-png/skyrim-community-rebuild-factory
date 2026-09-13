# Skyrim AE 1.7.104.0 — Address Library format 5 / REL failures

**Date:** 2026-09-13 (America/New_York)  
**Game:** Skyrim SE/AE `1.7.104.0` · SKSE `2.3.1` · `versionlib-1-7-104-0.bin` present (format **5**)  
**Scope:** Real restore paths for Object Impact Framework, Description Framework, Enchanting Cost Formula Fix; CIF vs OIF; other format-5 casualties Jo may hit next.

---

## Root cause (not “wrong Address Library file”)

Jo already has the correct Address Library binary for this runtime (`versionlib-1-7-104-0.bin`). That file is **format 5** (new with the 1.7.x Address Library drop — community notes call out AL **v12** for 1.7.99 and **v13** for 1.7.104).

The error:

- `Unsupported address library format: 5`
- `REL/... failed to open address library file` / format mismatch

means the **plugin DLL was linked against an older CommonLib** that only understands formats **1 / 2** (and VR), **not** `Format::SSEv5`.

Confirmed in current `alandtse/CommonLibVR` `ng` (`include/REL/IDDB.h` / `src/REL/IDDB.cpp`):

- Enum includes `SSEv1`, `SSEv2`, **`SSEv5`**, `VR`
- `load_v5(...)` + `header_v5_t`
- Exact user-facing string: `"Unsupported address library format: {}\n"`

Confirmed **absent** in the CommonLib pins used by the shipping mods:

| Mod | CommonLib pin | Formats known |
|-----|---------------|---------------|
| OIF (vcpkg port `REF e617713…`) | `alandtse/CommonLibVR` | SSEv1, SSEv2, VR only |
| Description Framework (`extern` @ `738afc4…`) | `alandtse/CommonLibVR` | SSEv1, SSEv2, VR only |

**Implication:** Renaming `.bin` files, reinstalling Address Library, or “just disable” does **not** fix this. Fix = **rebuild each DLL against a CommonLib that includes SSEv5** (current `alandtse/CommonLibVR` `ng` / CommonLibSSE-NG **≥ ~6.6–6.7**, which landed AE 1.7.99 runtime support). Same pattern as community Fleikon-style BTPS / DtryKeyUtil 1.7 patches: bump CommonLib, rebuild, drop new DLL.

**Infrastructure Jo already has (keep):**

- SKSE 2.3.1 for 1.7.104 — https://skse.silverlock.org/
- Address Library with `versionlib-1-7-104-0.bin` — https://www.nexusmods.com/skyrimspecialedition/mods/32444 (All-in-one / v13-class package)

---

## 1. Object Impact Framework (OIF) — Nexus 149484 — **PRIORITY RESTORE**

### Exact failure mode
`ObjectImpactFramework.dll` hard-fails at REL/Address Library load with **unsupported format 5** (or equivalent “failed to open address library”). Jo’s build: **OIF 1.6.8** (~Aug 2025), DLL `ObjectImpactFramework.dll`. Temporarily disabled; needs restore path.

### Compatible public build?
**No 1.7 / 1.7.97 / 1.7.99 / 1.7.104 build found** (Nexus files still top out at **1.6.8**; GitHub releases stop at **v1.6.8**, 2025-08-28).

Public downloads (pre-1.7, will still format-5 fail on 1.7.104):

- Nexus: https://www.nexusmods.com/skyrimspecialedition/mods/149484
- GitHub release DLL: https://github.com/arachnocid/ObjectImpactFramework/releases/download/1.6.8/ObjectImpactFramework.dll
- Tag/source: https://github.com/arachnocid/ObjectImpactFramework/releases/tag/1.6.8

Forks checked (`kuroishitam`, `QTR-Modding`): no clear published 1.7 binary / format-5 patch found.

### Source / rebuild feasibility
| Item | Detail |
|------|--------|
| Owner | **arachnocid** |
| License | **Apache-2.0** (rebuild/redistribute friendly) |
| Source | https://github.com/arachnocid/ObjectImpactFramework |
| Stack | CMake + vcpkg; depends on `commonlibsse-ng` overlay that pins **old** CommonLib (`e617713`, no SSEv5) |
| Feasibility | **High** — bump `cmake/ports/commonlibsse-ng/portfile.cmake` `REF` to current `alandtse/CommonLibVR` `ng` (post-SSEv5 / ≥6.6–6.7), rebuild AE preset, test hooks/JSON |

Rebuild may still need ID/offset touch-ups if any non-AddressLib hooks drifted 1.6.1170 → 1.7.104 (same class of work as Fleikon BTPS / open-shaders 1.7.99 PR). JSON rule packs themselves are fine.

### CIF does **not** replace OIF
**Core Impact Framework (CIF)** — Nexus https://www.nexusmods.com/skyrimspecialedition/mods/146873 (Seb263) — is a **combat/actor impact** overhaul (weapons, spells, blood, locational impacts).  

**OIF** is **world-object interaction** (Activate / Grab / Throw / Hit on clutter, flora, etc.) driven by `Data/SKSE/Plugins/ObjectImpactFramework/*.json`.

OIF’s own docs list CIF as a **sibling** (“for actors”), not a successor. **Jo cannot use CIF alone to restore OIF-dependent content.** Keep OIF (rebuilt) if those JSON packs matter; add CIF only if Jo wants CIF’s combat ecosystem.

CIF Nexus still shows **1.2.8** / **2.0.x** files; latest filename timestamps include **2026-02-15** (pre-1.7.99) and page noise around **2026-08-13**. Treat CIF as **suspect for the same format-5 failure** until a post-2026-08-20 build is confirmed — do not assume it is a free OIF substitute.

### Recommended action for Jo
1. **Keep OIF parked** (DLL disabled) so the game boots.
2. **Claude rebuild lane (preferred restore):** clone Apache-2.0 source → bump CommonLib port to SSEv5-capable `ng` → MSVC/CMake AE build → replace `ObjectImpactFramework.dll` → smoke-test activation/grab/throw JSON packs.
3. Watch Nexus/GitHub for an official **arachnocid** 1.7 drop; if it lands, prefer author DLL over local rebuild.
4. **Do not** install CIF expecting OIF features.

---

## 2. Description Framework — Nexus 105799 — **2.1.2**

### Exact failure mode
Same class: `DescriptionFramework.dll` (or similarly named) fails REL load with **unsupported address library format: 5**. Breaks description packs (`*_DESC.ini`).

### Compatible public build?
**No.** Nexus latest remains **2.1.2** (`Description Framework-105799-2-1-2-1756574770.rar`, uploaded **2025-08-30** — pre-1.7). No GitHub release assets for a 1.7 rebuild; forks (`clayne`, `rethesda`, `VelouriasMoon`) show no published format-5 binary.

- Nexus: https://www.nexusmods.com/skyrimspecialedition/mods/105799
- Source: https://github.com/Nightfallstorm/DescriptionFramework

### Source / rebuild feasibility
| Item | Detail |
|------|--------|
| Owners | **Nightfallstorm** (+ JaySerpa on Nexus page) |
| License | **GPL-3.0** |
| Build | CMake; submodule `extern/CommonLibSSE-NG` → `alandtse/CommonLibVR` @ **`738afc4`** (no SSEv5) |
| Open issues | Existing issues are feature/UX (`#1` Formatting, `#2` Log, `#3` enchanting descriptions) — **no open 1.7/format-5 ticket found** |
| Feasibility | **High** — update submodule to current `ng` with SSEv5, rebuild; UI menu hooks may need 1.7 validation |

### Recommended action for Jo
1. Park DLL for now (description packs inert without framework).
2. **Claude rebuild lane** (GPL-3.0 — fine for private use; redistribution must stay GPL).
3. Optionally open a GitHub issue / Nexus post asking Nightfallstorm for an official 1.7.104 build.
4. After rebuild: re-enable framework + Jo’s description packs; verify inventory/enchant UI text.

---

## 3. Enchanting Cost Formula Fix — Nexus 62144

### Exact failure mode
SKSE **2.0** remake (Aug 2025) fails the same **format 5** REL check. Older **1.x** variants were **ESP/game-setting** tweaks (not DLL) — different failure profile.

### Compatible public build?
**No 1.7 build found.** Nexus still hosts **2.0** from ~2025-08-06 (Reddit announce by u/Nice_Association1655). Requires Address Library per page copy.

- Nexus: https://www.nexusmods.com/skyrimspecialedition/mods/62144

### Source / rebuild feasibility
| Item | Detail |
|------|--------|
| Author | Nexus / Reddit **Nice_Association1655** |
| Public source | **Not found** (no GitHub repo located) |
| Feasibility | **Low without source** — wait for author, or reimplement formula via ESP/game settings (1.x approach) / Scrambled Bugs enchantment cost pieces |

### Recommended action for Jo
**OK to skip** as Jo indicated. Prefer:

- Keep **Scrambled Bugs** enchantment-cost fixes if already in list (Nexus 43532 / related article), and/or  
- Fall back to non-DLL **1.x Fixed Formula** style (`fEnchantingSkillCostBase` tweak) if that still meets the design goal  

Only chase a Claude rebuild if author publishes source or Jo explicitly wants the 2.0 SKSE formula.

---

## 4. Core Impact Framework vs OIF (summary)

| | **OIF** | **CIF** |
|--|---------|---------|
| Nexus | 149484 | 146873 |
| Author | arachnocid | Seb263 |
| Domain | Object interact (activate/grab/throw/hit JSON) | Actor/combat impacts |
| Replaces the other? | **No** | **No** |
| Alone enough for Jo’s OIF packs? | Required | **No** |
| 1.7.104 status | Broken format 5; rebuild | Likely same class until post-1.7 DLL confirmed |

---

## Rebuild playbook (Fleikon BTPS-class)

What worked for BTPS (and the documented DtryKeyUtil 1.7 community patches) applies here:

1. Confirm `versionlib-1-7-104-0.bin` stays installed (do **not** replace with an older format).
2. Clone mod source; point CommonLib at **`alandtse/CommonLibVR` `ng`** revision that contains `Format::SSEv5` / `load_v5` (CommonLibSSE-NG **6.6+ / 6.7** era for 1.7.99+).
3. Build AE (or NG flatrim) with VS 2022 + vcpkg.
4. Drop new DLL over the old one; keep JSON/INI assets.
5. First boot: watch `Documents\My Games\Skyrim Special Edition\SKSE\<Mod>.log` and SKSE plugin loader for remaining missing-ID errors (those need RelocationID updates, not more Address Library files).

Reference pattern (not a drop-in for OIF/DF): community **DtryKeyUtil-1.7.104-Patch** class rebuilds; alandtse open-shaders PR noting CommonLib bump to **v6.7.0** for AE 1.7.99.

---

## Other SKSE plugins Jo may hit next (same format-5 / 1.7.104 class)

Any DLL still linked to pre-SSEv5 CommonLib will die the same way even with a correct `versionlib-1-7-104-0.bin`. Highest-risk names called out in Aug/Sep 2026 community notes:

| Plugin | Why it matters | Notes |
|--------|----------------|-------|
| **RaceMenu** (`skee64.dll`) | Character menus / bodygen | Official Nexus still oriented to 1.6.1170; no confirmed 1.7.104 file in surveyed writeups |
| **SSE Engine Fixes** | Deep engine hooks | 7.0.21 beta called out for **1.7.99 only**; confirm explicit **1.7.104** before trusting |
| **Better Third Person Selection** | Already mitigated for Jo via **Fleikon 1.7.99 GitHub patch** | Re-verify on **1.7.104** (offsets may differ from 1.7.99) |
| **dTry Key Utils** / combat DLLs | Key/combat stacks | Needs community **1.7.x** rebuilds (e.g. DtryKeyUtil-1.7.104-Patch class), not original Nexus DLL |
| **po3 stack** (Tweaks, Papyrus Extender, SPID, KID, BOS, Grab and Throw) | Soft deps for OIF/DF | Usually updated quickly; still verify each DLL date ≥ 2026-08-20 |
| **JContainers**, **MCM Helper**, **PapyrusUtil** | Framework glue | Same “DLL must understand format 5” rule |
| **Precision / True Directional Movement / TrueHUD / OAR** | Combat/UI | Popular; check author 1.7 files before assuming NG immortality |
| **CIF** (if Jo installs later) | Combat impacts | Separate from OIF; likely needs its own 1.7 DLL confirmation |

**Heuristic:** In MO2 Data tab, filter `dll`. Any SKSE plugin whose author last shipped before ~2026-08-20 is guilty until proven format-5-aware.

---

## Priority action plan for Jo

| Priority | Mod | Action |
|----------|-----|--------|
| P0 | **OIF** | Claude rebuild lane (Apache-2.0, source ready); restore JSON packs after new DLL |
| P0 | **Description Framework** | Claude rebuild lane (GPL-3.0); restore description packs |
| P1 | **Enchanting Cost Formula Fix** | Skip / park; no public source; use Scrambled Bugs / ESP formula if needed |
| P1 | **CIF** | Not an OIF replacement; only install if combat impacts wanted **and** 1.7 DLL confirmed |
| Ongoing | Other DLLs | Audit `Data/SKSE/Plugins`; update or rebuild anything still throwing format 5 |

**Success criteria met:** actionable restore paths = **rebuild against SSEv5 CommonLib** for OIF + Description Framework; Enchanting Cost = skip-with-alternatives; CIF clarified as non-replacement.

