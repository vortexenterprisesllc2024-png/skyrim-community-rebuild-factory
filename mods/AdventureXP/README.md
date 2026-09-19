# AdventureXP 4.2

A **brand-new**, clean-room SKSE plugin for *The Elder Scrolls V: Skyrim Special Edition* (Anniversary Edition **1.7.104.0**). It awards **player level XP** for finishing quests, discovering locations, and clearing dungeons — adventure-first progression rather than grinding a skill.

**Version 4.2.3** keeps the 4.2.1 Reading/Combat ignore-global rule and the 4.2.2 reading formula `floor(sqrt(bookGold) * fReadingMult)`. Quest stage and quest complete also ignore global (`base * categoryWeight/100`) so `iObjectives=12` at `fGlobalXPPercent=2` awards visible `+12 XP`, not `+0`. `bAwardQuestStages` stays on. `Awards::Give` still adds fractional XP to the pool, but suppresses the on-screen toast when `lround(amount) < 1`. Visible toasts print the rounded integer with `%d`. Notes/letters with gold 0 stay 0. Flat `fReadingXP` remains in the INI as an unused legacy key. Discovery, clears, skill-ups, and crafting still multiply by global. It is not a continuation of anyone else’s 3.x package.

Similar Skyrim mods exist. **Every line of code and every asset in this archive is newly written.** Nothing here is copied, forked, or derived from other XP plugins or their source. Credit is not permission. See `CLEANROOM.md`.

## What it does

1. Awards player XP into an adventure pool (quests, discovery, clears, optional combat / reading / crafting / skill-ups).
2. Play-style presets write category sliders, quest/place amounts, and per-skill weights.
3. Global XP percent plus per-quest and per-place **absolute XP** (0–200). Per-skill weights are 0–100%. Quest, Reading, and Combat ignore global XP percent and use category weight only. Reading base XP is `floor(sqrt(goldValue) * fReadingMult)` (default mult 1.0). Quest stages stay enabled.
4. On-screen XP toast via `DebugNotification` (no third-party HUD SWF).
5. `AdventureXP_Percent` global in `AdventureXP.esl` for other HUDs.
6. **SkyUI MCM named AdventureXP** — enable/disable, pick a pack, edit every category. Calls only `AdventureXP.*` natives.

## Requirements (in-game)

- Skyrim SE AE **1.7.104.0**
- [SKSE](https://skse.silverlock.org/) **2.3.1**
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [SkyUI](https://www.nexusmods.com/skyrimspecialedition/mods/12604) **5.2 SE** (MCM only; the DLL still awards XP without it)
- `AdventureXP.dll` built on Windows from this tree (this pack ships Papyrus + INI + ESL + C++ source)

## Install (Vortex / MO2)

Packed zip name: **`AdventureXP-4.2.3.zip`**. Data-root layout:

```
Data/AdventureXP.esl
Data/AdventureXP.seq
Data/SKSE/Plugins/AdventureXP.dll
Data/SKSE/Plugins/AdventureXP.ini
Data/Scripts/Source/AdventureXP.psc
Data/Scripts/Source/AdventureXPMCM.psc
Data/Scripts/AdventureXPMCM.pex   (after you compile — see below)
Data/Scripts/AdventureXP.pex
```

Enable `AdventureXP.esl` and SkyUI. Launch through `skse64_loader.exe`.

In-game: **Pause → Mod Configuration → AdventureXP**.

Logs: `Documents/My Games/Skyrim Special Edition/SKSE/AdventureXP.log`

A mid-playthrough install **does not** dump XP for dungeons you already cleared. `AdventureXP.seq` starts the MCM quest on existing saves.

## SkyUI MCM

The menu is `AdventureXPMCM` on quest `AdventureXPMCMQuest`. MCM script `GetVersion()` is **5**.

| Page | Options |
| --- | --- |
| **General** | Award Adventure XP; Show Messages; Global XP (0–100%); Playstyle Pack |
| **XP Sources** | Enable Killing; Enable Reading; Quests / Discovery / Clears / Combat / Reading / Crafting / Skill-ups (0–100%) |
| **Quest Types** | Quest Objectives, Quest None, Main, College, Thieves, Brotherhood, Companions, Misc, Daedric, Side, Civil War, Dawnguard, Dragonborn (0–200 XP) |
| **Discovery** | 39 place kinds, discover XP 0–200 |
| **Clears** | The same 39 place kinds, clear XP 0–200 |
| **Skills** | 18 vanilla skills in Magicka / Combat / Stealth groups (0–100%). Master Skill-ups at 0 turns all of these off. |

There is no XP meter in AdventureXP, so there is no meter-mode control.

College / Thieves / Brotherhood / Companions are separate. Dawnguard and Dragonborn are separate. Discovery and clear amounts are separate for every place kind.

Place kinds: Altar, Camp, Castle, Castle Karstaag, Cave, City, Clearing, Daedric Shrine, Default, Docks, Doomstone, Dragon Lair, Dwemer Ruin, Farm, Fort, Giant Camp, Grove, Imperial Tower, Landmark, Lighthouse, Military Camp, Mine, Miraak Temple, Nordic Dwelling, Nordic Ruin, Nordic Tower, Orc Stronghold, Pass, Rock, Settlement, Shack, Shipwreck, Smelter, Stable, Standing Stone, Telvanni Tower, Town, Wheat Mill, Wood Mill.

How a location is sorted is documented in `docs/PLACE_MAPPINGS.md`.

### Persistence — no reload

The DLL owns state and `AdventureXP.ini`. Every MCM control calls a native. Changes apply immediately. You do not quit to the main menu. The award toggle uses `SetToggleOptionValue` plus `SetOptionFlags` on the other General controls only. It never calls `ForcePageReset`, so you can turn awards back on without leaving General.

### Packs in the MCM vs the INI

They are the same table.

- INI `sPreset=Thief` applies that pack on load.
- MCM **Playstyle Pack** calls `AdventureXP.ApplyPreset("Thief")`.
- Applying a pack writes the **full role-play table**: category masters, every quest type, discover/clear place biases, per-skill weights, skip-misc, main-quest multiplier, and undead/stealth/beast flavor.
- After a pack is applied you can still nudge sliders. Editing a slider sets `sPreset=Custom` so the next load does not overwrite you.

Papyrus:

```
AdventureXP.ApplyPreset("Thief")
AdventureXP.GetCategoryWeight("discovery")
AdventureXP.SetCategoryWeight("combat", 80.0)
AdventureXP.SetQuestXP("College", 90)
AdventureXP.SetDiscoveryXP("Cave", 30)
AdventureXP.SetClearXP("Cave", 100)
AdventureXP.GetSkillWeight("Illusion")
AdventureXP.SetSkillWeight("Sneak", 80.0)
```

## Rebuild the DLL (Windows)

Linux host tests compile the classifiers only. **The live game still needs a Windows MSVC rebuild of `AdventureXP.dll`** before the new natives (`GetSkillWeight`, `SetSkillWeight`, plus quest/place/message/killing/reading) exist in-game.

1. Visual Studio 2022 (x64) and [vcpkg](https://github.com/microsoft/vcpkg).
2. Install CommonLibSSE-NG through vcpkg (`vcpkg.json` in this tree).
3. `cmake --preset windows-msvc-release` then build `AdventureXP`.
4. Copy `AdventureXP.dll` to `Data/SKSE/Plugins/`.
5. Compile the two Papyrus scripts in the Creation Kit (or Caprica) so SkyUI can load `AdventureXPMCM.pex`.

Event sink type names follow CommonLibSSE-NG. If your CommonLib revision names a sink differently, adjust `src/Events.cpp` only — `Types.h` / Config / Papyrus stay the same.

## Compile the MCM (Creation Kit)

This pack ships `.psc` sources. SkyUI needs `.pex` on disk. Compile once on a Windows machine that has the Creation Kit and SkyUI:

1. Install SkyUI so `SKI_ConfigBase.pex` is in `Data/Scripts`.
2. Copy `Scripts/Source/AdventureXP.psc` and `AdventureXPMCM.psc` into the CK source folder (`Source/Scripts` on SE).
3. Compile **AdventureXP** (hidden native stub) then **AdventureXPMCM**.
4. Copy the two `.pex` files to `Data/Scripts`.

Caprica (optional): point `--import` at vanilla + SkyUI sources, compile the same two scripts, do **not** ship SkyUI’s own scripts.

Until `.pex` exists, SkyUI will not list AdventureXP. The plugin DLL still runs from the INI.

## Play-style packs

Set `sPreset` or pick the same id in the MCM (case-insensitive):

| Pack | Pays | Denies |
| --- | --- | --- |
| **Adventurer** | Quests, discovery, clears at stock amounts | Combat, reading, crafting, skill-ups |
| **Vigilant** | Barrow clears, undead, scripture, Main, Dawnguard; Restoration ranks | Daedric = 0, Thieves/Brotherhood, busywork Misc |
| **Summoner** (shows as Conjurer) | College, reading, Conjuration 100 | Combat, fortress clears, other schools low |
| **Necromancer** (shows as Summoner) | Tombs, reading, Conjuration / Enchanting, Daedric medium | Fighting in person, military camps |
| **Thief** | Discovery, Thieves Guild; Sneak / Lockpicking / Pickpocket / Speech 100 | Clears, Civil War, Main |
| **Assassin** | Brotherhood, Sneak / One-handed / Archery | Other skills 0, reading, crafting |
| **Paladin** | Main, holy clears, Restoration / One-handed / Block / Heavy | Daedric = 0, Thieves = 0, Brotherhood = 0 |
| **Warrior** | Clears, kills, Companions, Civil War; One-handed / Two-handed / Block / Heavy / Smithing 100 | Reading, College, magic schools 0 |
| **Mage** | College, reading; all six magic schools 100 | Combat skills 0, fort clears |
| **Ranger** | Woods/camps/passes, Archery / Sneak / Light Armor | City Misc, hold capitals |
| **Bard** | Side, Misc stories, Speech 100 | Clears, shield-wall combat |
| **Merchant** | Crafting, Speech / Smithing / Alchemy | Combat = 0, clears = 0 |
| **Beastblood** | The hunt, clears, Two-handed / Light Armor | Books, the forge, Misc busywork |
| **Spellsword** | Combat + College + clears; Destruction / One-handed mixed | None — mixed war-mage |
| **Monk** | Training, pilgrimage stones/altars; Restoration / Alteration | Crafting, Brotherhood, Thieves |
| **Illusionist** | Illusion 100, Speech medium, College | Other magic schools 0, combat skills 0 |
| **elementalist** (shows as Elementalist) | Destruction 100, College | Other schools 0 |
| **Battlemage** | Destruction + Conjuration 100, One-handed / Heavy Armor mixed | Stealth skills 0 |

MCM labels: Necromancer → Summoner, Summoner → Conjurer. `ApplyPreset("Conjurer")` still selects the Summoner id.

## Pack zips

```bash
python3 scripts/generate_esl.py
python3 scripts/pack.py --allow-missing-dll --source
```

Writes:

- `dist/packed/AdventureXP-4.2.3.zip` — Vortex Data-root layout (DLL included when present)
- `dist/packed/AdventureXP-4.2.3-SOURCE.zip` — this tree

## Host tests

```bash
scripts/host-test.sh
```

Covers C++ classifiers + INI load/save, ESL records, MCM-only-uses-`AdventureXP.*`, schema lockstep (39 places / 13 quest kinds / 18 skills), playstyle deny zeros (Illusionist skills, Vigilant Daedric), zip layout, and third-party filename / symbol asserts.

## License

- **AdventureXP sources:** MIT (`LICENSE`).
- A distributed `AdventureXP.dll` that links CommonLibSSE-NG is a GPL work.
