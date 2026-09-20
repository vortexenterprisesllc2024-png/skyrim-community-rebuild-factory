# AdventureXP 4.2.11 — journal-visible quest stages

Drop-in replacement for 4.2.10. Same factory DLL (`AdventureXP.dll` in `Data/SKSE/Plugins`). Keep the existing ESL / INI / scripts unless you want the new default INI comment.

## What changed

`bAwardQuestComplete` is unchanged: finishing a quest still awards the quest-type XP.

`bAwardQuestStages` no longer pays on every `TESQuestStageEvent`. 4.2.10 toasted on each stage != 0 (once per formID+stage), including empty/startup/script-only stages. Climbing the 7000 Steps during **The Way of the Voice** fired several of those and stacked `+6 XP (quest objective)` with no journal beat. Hiding the toast would still move the bar. 4.2.11 withholds the award unless the player can see an objective / journal advance.

## How the filter decides

Evaluated one main-thread tick after the stage event so CK/Papyrus fragments (`SetObjectiveDisplayed`, journal CNAM) have run.

1. **Always skip stage 0** (quest start / empty). Even with the silent-stage override.
2. **Once per formID+stage** in this session (same as 4.2.10’s described cap).
3. **Skip `kStartUpStage` / `kShutDownStage`** when those flags are present on the `TESQuestStage`. Shutdown completion still pays through `bAwardQuestComplete`.
4. **Award if the instance journal is this stage and has visible text.** `BGSQuestInstanceText.journalStage` must match the event stage, `journalStageItem >= 0`, and `TESQuest::GetJournalTextForInstance` returns non-whitespace text. A silent trigger leaves `journalStage` on the last real log entry.
5. **Or award if a journal objective advanced.** Walk `TESQuest::objectives` (`BGSQuestObjective`). The NNAM display text must be non-empty, and the state must *change* into `kDisplayed` / `kCompletedDisplayed` / `kFailedDisplayed`. Script-only `kCompleted` / `kFailed` (not displayed) do not count.

**Discarded guess:** “award if the quest already has any displayed objective.” Way of the Voice keeps *Speak to the Greybeards* displayed the whole climb. That test would still ping on the silent steps.

On `kDataLoaded` / `kNewGame` / `kPostLoadGame` the plugin snapshots current objective states so an already-visible line from the save does not look like an advance.

Toasts use the quest’s full name (`TESQuest::GetName` / `GetFullName`). If the name is empty, they fall back to the objective NNAM that just advanced.

Skipped silent stages are logged:

```text
Quest stage XP skipped (not journal-visible): The Way of the Voice stage 15
```

`Documents/My Games/Skyrim Special Edition/SKSE/AdventureXP.log`

## INI knobs (`[Quests]`)

| Key | Default | Role |
| --- | --- | --- |
| `bAwardQuestStages` | `1` | Master gate for per-stage / objective XP (`iObjectives`) |
| `bAwardQuestComplete` | `1` | Full quest-complete XP by CK type |
| **`bAwardSilentQuestStages`** | **`0`** | `0` = journal-visible filter. `1` = 4.2.10 every stage != 0 |
| `bSkipHiddenQuests` | `1` | Skip unnamed quests |
| `iObjectives` | `12` | Absolute XP per awarded stage (then `fQuestWeight/100`; ignores `fGlobalXPPercent`) |

Existing 4.2.10 INIs that omit `bAwardSilentQuestStages` get the new default (`0`). Set it to `1` if you want the old spam back.

## Playtest checklist

Replace `Data/SKSE/Plugins/AdventureXP.dll` (and the INI if you want the new comment). SKSE-boot 1.7.104.

1. **7000 Steps / Way of the Voice** — walk the emblems to High Hrothgar. You should **not** get a stack of quest toasts and the XP bar should **not** creep on those silent stages. `AdventureXP.log` may show “skipped (not journal-visible)” lines.
2. **Real journal beat** — finish or display a visible objective (talk to someone that updates the journal, check off a listed task). You **should** get `+N XP (Quest Name)` and the bar should move.
3. **Quest complete** — finish a named quest. You should still get the complete-type award (`Main`, `Side`, …).
4. **Override** — set `bAwardSilentQuestStages=1`, reload the save / restart, climb again. The old every-stage pings should return.
5. **Unchanged 4.2.10 paths** — discovery / clears / reading `sqrt(gold)` / skill-up scale / +0 toast suppress / Crafter & Wizard packs / vanilla XP-bar sync. Global still unused for awards.

If (1) still pings, grab the log lines for those stages (quest name + stage index) before filing a follow-up.
