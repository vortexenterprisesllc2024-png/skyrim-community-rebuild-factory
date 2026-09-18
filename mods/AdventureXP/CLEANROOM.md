# Clean-room rules (AdventureXP 4.0)

Nexus removed the unofficial Experience AE page because zax’s Experience (17751) forbids upload, modification, and asset use. **Credit is not permission.**

**Clean-room keep from those packages: 0 files.** Nothing from Experience zips ships here.

## Forbidden (do not copy, vendor, or rename-into-tree)

| Kind | Examples |
| --- | --- |
| ZAX_ORIGIN | `Experience.dll`, `HUDHooks.swf`, `statsmenu.swf`, `trainingmenu.swf`, `Experience.pex` / `Experience.psc`, `Actors/Default.ini`, `Races/Default.ini`, zax `LICENSE.txt` |
| JO MCM that calls zax natives | `ExperienceMCM.*`, `ExperienceMCM_SkillXP.*`, `ExperienceMCM_Reset.*`, `ExperienceMCM_Save.*` — rewrite against `AdventureXP` Papyrus only |
| Mixed | `Experience.ini` (zax setting schema), any `Experience.dll` rebuild |

`scripts/assert-cleanroom.py` fails the host tests if those names appear as shipped files.

## What this repo ships instead

- Newly written C++ (`AdventureXP.dll` target), INI, ESL, MIT `LICENSE`, and `AdventureXP.psc` natives.
- Play-style packs and quest/place sliders designed here. Inspired by EverQuest-style leveling and other games; similar Skyrim mods exist.
- No SWF HUD. Toasts use vanilla `DebugNotification`.

## Names

`AdventureXP.dll`, `AdventureXP.esl`, `AdventureXP.ini`, editor ID `AdventureXP_Percent`. Never `ExperienceMCMQuest` / `ExperienceMCM`.
