# AdventureXP 4.0

Working title. Public name is TBD.

A **brand-new**, clean-room SKSE plugin for *The Elder Scrolls V: Skyrim Special Edition* (Anniversary Edition **1.7.104.0**). It awards **player level XP** for finishing quests, discovering locations, and clearing dungeons — the same kind of adventure-first progression you see in EverQuest and other RPGs, rather than leveling up because you ground a skill.

**Version 4.0.0** is a new product line. It is not a continuation of anyone else’s 3.x package.

Similar Skyrim mods exist. **Every line of code and every asset in this archive is newly written.** Nothing here is copied, forked, or derived from zax’s Experience packages or source. Credit is not permission.

## What it does

1. **Ignores vanilla skill-up leveling** when configured, by setting the stock game setting `fXPPerSkillRank` to `0` at runtime. Skills still improve; they just stop feeding the vanilla player-XP formula. Packs that want training XP use AdventureXP’s own skill-up / crafting sliders.
2. **Awards player XP** into the vanilla `PlayerSkills` pool so the stock level-up / perk-point flow still works:
   - quest stage updates and quest completion
   - map-marker location discovery
   - locations the game marks **cleared**
   - optional combat, reading, crafting ranks, and other skill-ups (driven by the active pack)
3. **Global XP percent** (`fGlobalXPPercent`).
4. **On-screen XP toast** via `DebugNotification` (no third-party HUD SWF).
5. **`AdventureXP_Percent` global** in `AdventureXP.esl` so other mods / HUD widgets can read 0–100 progress.
6. **Play-style presets** — fifteen named packs that move the real XP category sliders (not cosmetic labels). INI `sPreset=` applies a pack on load. The same `ApplyPreset()` table is exposed to Papyrus so SkyUI MCM can do it in one click later.

Out of scope for this slice: a finished SkyUI menu, HUDHooks / stats-menu replacements, compatibility patches for other leveling mods, and any Nexus upload.

## Play-style packs (4.0)

Set `sPreset` in `AdventureXP.ini` to one of these ids (case-insensitive). Each pack writes **quests / discovery / clears / combat / reading / crafting / skill-ups** plus a small flavor bonus where it matters (undead, sneak, hunt).

| Pack | What actually changes |
| --- | --- |
| **Adventurer** | Default EverQuest-style diet: quests, discovery, clears. Combat/reading/training off. |
| **Vigilant** | Clears and undead kills up; scripture (reading); discovery down. |
| **Summoner** | Reading and school skill-ups up; you fight less. |
| **Thief** | Discovery and stealth skill-ups up; clears down; sneak-kill bonus. |
| **Assassin** | Combat way up, reading/crafting down; large sneak-kill bonus. |
| **Paladin** | Quests and clears up; undead bonus; main-quest multiplier high. |
| **Warrior** | Clears, kills, and the forge (crafting / martial skill-ups). |
| **Mage** | Reading, enchanting/alchemy, and school ranks. |
| **Ranger** | Discovery first, then bow kills and camps. |
| **Bard** | Quests, the road, speech-heavy skill-ups, songs (reading). |
| **Merchant** | Crafting + speech skill-ups + jobs; combat almost off. |
| **Necromancer** | Tombs, treatises, and the dead (clear + reading + undead bonus). |
| **Beastblood** | Hunt and clears; books off; beast/animal kill bonus. |
| **Spellsword** | Balanced war-mage: combat + training + clears. |
| **Monk** | Pilgrimage and training; crafting off; restrained kill weight. |

`sPreset=Custom` skips the pack table and uses whatever is in `[Weights]` / `[Flavor]`. After a named pack loads, those same sliders can still be overridden in the INI.

Papyrus (for a future MCM, not shipped as a compiled menu):

```
AdventureXP.ApplyPreset("Thief")
AdventureXP.GetCategoryWeight("discovery")
AdventureXP.SetCategoryWeight("combat", 80.0)
```

Source: `scripts/papyrus/AdventureXP.psc`. Newly written. It does not call any third-party natives.

## Requirements (in-game)

- Skyrim SE AE **1.7.104.0**
- [SKSE](https://skse.silverlock.org/) **2.3.1**
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) **format 5**
- The plugin DLL is **4.0.0** and declares Address Library compatibility (CommonLibSSE-NG v8, including the v5 flag)

## Install (Vortex / MO2)

Packed zip name: **`AdventureXP-4.0.0.zip`**. Data-root layout:

```
Data/AdventureXP.esl
Data/SKSE/Plugins/AdventureXP.dll
Data/SKSE/Plugins/AdventureXP.ini
```

Enable `AdventureXP.esl`. Launch through `skse64_loader.exe`.

Logs: `Documents/My Games/Skyrim Special Edition/SKSE/AdventureXP.log`

A mid-playthrough install **does not** dump XP for dungeons you already cleared.

## Configure

| Key | Meaning |
| --- | --- |
| `sPreset` | Pack id, or `Custom` |
| `fGlobalXPPercent` | Multiplier for every award |
| `fQuestWeight` … `fSkillUpWeight` | Category sliders (100 = the base XP next to them) |
| `bIgnoreSkillLeveling` | Zero `fXPPerSkillRank` |
| `fUndeadCombatBonus` / `fStealthCombatBonus` / `fBeastCombatBonus` | Extra % on matching kills |

Base XP amounts (`fQuestCompleteXP`, `fDiscoveryXP`, …) stay in the INI so a 150 combat weight means “150% of `fKillXP`”, not a hidden second table.

## Build (Windows MSVC) — produces `AdventureXP.dll` 4.0.0

CommonLibSSE-NG is MSVC-ABI. This Linux agent box and Taquitos (no VS 2022) cannot link a loadable SKSE plugin. Factory / a VS 2022 machine:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build-vs2022.ps1
```

That script finds VS 2022, bootstraps vcpkg if needed, builds the DLL, and writes `dist\packed\AdventureXP-4.0.0.zip`.

Manual equivalent:

```bat
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
git -C C:\vcpkg checkout ee12231b20c95013c6638d845d04c91559a1d1ff
C:\vcpkg\bootstrap-vcpkg.bat
set VCPKG_ROOT=C:\vcpkg

cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release --parallel
python scripts\pack.py --source
```

CMake FetchContent-pulls [alandtse/CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG) **v8.0.1**. `add_commonlibsse_plugin` embeds product version **4.0.0** in the DLL.

A GitHub Actions workflow (`.github/workflows/build.yml`) builds the Windows DLL and uploads both zips. That job does not run on this Origin remote — point a GitHub repo at the same tree, or run `scripts\build-vs2022.ps1` on a VS 2022 machine.

## Pack zips

```bat
python scripts\generate_esl.py
python scripts\pack.py --source
```

Writes:

- `dist/packed/AdventureXP-4.0.0.zip` — Vortex Data-root layout (DLL included when MSVC built it)
- `dist/packed/AdventureXP-4.0.0-SOURCE.zip` — full clean-room source tree

Layout-only (no DLL on this machine):

```bash
python3 scripts/pack.py --allow-missing-dll --source
```

## Host tests

```bash
cmake --preset host-tests
cmake --build --preset host-tests
ctest --preset host-tests --output-on-failure
python3 scripts/generate_esl.py
python3 tests/test_esl.py
python3 tests/test_pack_layout.py
python3 scripts/assert-cleanroom.py
```

Or `scripts/host-test.sh`, which also writes the official layout + source zips.

Covers XP math, the 15 pack signatures (they must differ), ESL, zip layout, and Experience/zax filename asserts.

See `CLEANROOM.md`. Host tests run `scripts/assert-cleanroom.py` so Experience/zax filenames cannot sneak into the tree.

## License

- **AdventureXP sources:** MIT (`LICENSE`).
- **CommonLibSSE-NG** is GPL-3.0-or-later. A distributed `AdventureXP.dll` that links it is a GPL work.

## Phase 2

SkyUI MCM named **AdventureXP**: a dropdown of the same pack ids and the seven category sliders, calling `AdventureXP.ApplyPreset`. No MCM menu is shipped in 4.0.0 so the plugin compiles without Papyrus Studio.
