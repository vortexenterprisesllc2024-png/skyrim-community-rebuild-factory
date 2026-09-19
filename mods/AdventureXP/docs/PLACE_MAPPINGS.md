# Place classification (AdventureXP 4.1)

Vanilla locations are sorted into 39 place kinds. Order of checks:

1. **Editor ID hints** for unique sites and kinds with no `LocType*` / no compass marker.
2. **`LocType*` keywords**, most specific first.
3. **Map marker type** (vanilla compass icon byte) when keywords do not decide.
4. **Default**.

Grouped only when Skyrim has no distinct keyword. Those rows are marked **grouped**.

## Keyword → kind

| Kind | Vanilla keywords |
| --- | --- |
| DragonLair | `LocTypeDragonLair`, `LocTypeDragonPriestLair` |
| GiantCamp | `LocTypeGiantCamp` |
| MilitaryCamp | `LocTypeMilitaryCamp` |
| OrcStronghold | `LocTypeOrcStronghold` |
| Mine | `LocTypeMine` |
| WoodMill | `LocTypeLumberMill` |
| Grove | `LocTypeSprigganGrove` |
| Shipwreck | `LocTypeShipwreck`, `LocTypeShip` |
| NordicRuin | `LocTypeNordicRuin` |
| DwemerRuin | `LocTypeDwarvenAutomatons` (there is no `LocTypeDwemerRuin`) |
| Cave | `LocTypeCave` |
| Fort | `LocTypeFort`, `LocTypeMilitaryFort` |
| Camp | `LocTypeCamp`, `LocTypeBanditCamp`, `LocTypeForswornCamp` |
| City | `LocTypeCity` |
| Town | `LocTypeTown` |
| Settlement | `LocTypeSettlement` |
| Farm | `LocTypeFarm` |
| Clearing | `LocTypeClearing` |
| Castle | `LocTypeCastle` when present |

`LocTypeDungeon`, `LocTypeAnimalDen`, `LocTypeDraugrCrypt`, and similar generic dungeon tags do **not** pick a kind by themselves. They fall through to the marker, then Default.

## Marker → kind (when keywords are Default)

Vanilla `MARKER_TYPE` bytes:

| Kind | Marker |
| --- | --- |
| City | City, plus hold *Capitol* markers |
| Town | Town, Raven Rock |
| Settlement | Settlement |
| Cave | Cave |
| Camp | Camp |
| Fort | Fort |
| NordicRuin | Nordic ruin |
| DwemerRuin | Dwemer ruin |
| Shipwreck | Shipwreck |
| Grove | Grove |
| Landmark | Landmark |
| DragonLair | Dragon lair |
| Farm | Farm |
| WoodMill | Wood mill |
| Mine | Mine |
| ImperialTower | Imperial tower |
| Clearing | Clearing |
| Pass | Pass |
| Altar | Altar |
| Rock | Rock |
| Lighthouse | Lighthouse |
| OrcStronghold | Orc stronghold |
| GiantCamp | Giant camp |
| Shack | Shack |
| NordicDwelling | Nordic dwelling |
| Docks | Docks |
| DaedricShrine | Shrine |
| Castle | Hold *Castle* markers |
| MiraakTemple | Miraak temple |
| Doomstone | Solstheim All-Maker stone |
| TelvanniTower | Telvanni tower |
| CastleKarstaag | Castle Karstaag |

Travel markers (to Skyrim / to Solstheim) stay **Default**.

## Grouped — no distinct LocType*

| Kind | How it is chosen | Grouped with / notes |
| --- | --- | --- |
| Altar | Marker Altar | No `LocTypeAltar` |
| CastleKarstaag | Marker + editor ID `Karstaag` | Unique DLC2 site |
| DaedricShrine | Marker Shrine, or editor ID `DaedricShrine` / `ShrineOf` | No `LocTypeDaedricShrine`. `LocTypeTemple` alone is not enough (covers Divine temples too) |
| Docks | Marker Docks | No `LocTypeDocks` |
| Doomstone | All-Maker marker, or editor ID `Doomstone` / `AllMaker` | Solstheim stones. Skyrim’s 13 guardian stones are StandingStone |
| ImperialTower | Marker Imperial tower | No `LocTypeImperialTower` |
| Landmark | Marker Landmark | No `LocTypeLandmark` |
| Lighthouse | Marker Lighthouse | No `LocTypeLighthouse` |
| MiraakTemple | Marker + editor ID `Miraak` | Unique DLC2 site |
| NordicDwelling | Marker Nordic dwelling | No `LocTypeNordicDwelling` |
| NordicTower | Editor ID `NordicTower` / `Watchtower` | **Grouped** with NordicRuin when that hint is missing |
| Pass | Marker Pass | No `LocTypePass` |
| Rock | Marker Rock | No `LocTypeRock` |
| Shack | Marker Shack | No `LocTypeShack` |
| Smelter | Editor ID `Smelter` | **Grouped** with Default when the name is missing |
| Stable | Editor ID `Stable` / `HorseMarker` | **Grouped** with Farm/Default when the name is missing |
| StandingStone | Editor ID `StandingStone` / `GuardianStone` | No `LocTypeStandingStone` |
| TelvanniTower | Marker + editor ID `Telvanni` | Unique DLC2 site |
| WheatMill | Editor ID `WheatMill` / `GrainMill` / `FlourMill` | **Grouped** with Farm when the name is missing |
| Default | Fallback | Generic `LocTypeDungeon` and unknown markers |

## Quest types

CK `TESQuest` type byte is used first:

| Kind | CK type |
| --- | --- |
| None | 0 |
| Main | 1 |
| College | 2 |
| Thieves | 3 |
| Brotherhood | 4 |
| Companions | 5 |
| Misc | 6 |
| Daedric | 7 |
| Side | 8 |
| CivilWar | 9 |
| Dawnguard | 10 |
| Dragonborn | 11 |

When the type byte is 0, editor-ID prefixes (`MG`, `TG`, `DB`, `C0x`, `DLC1`, `DLC2`, …) and guild faction editor IDs are used. College / Thieves / Brotherhood / Companions are never collapsed into one Guild slider. Dawnguard and Dragonborn are never collapsed into one DLC slider.
