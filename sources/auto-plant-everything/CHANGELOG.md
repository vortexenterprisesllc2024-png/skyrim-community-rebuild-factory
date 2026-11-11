# Changelog

## 1.0.0 — 2026-11-11
- Runs after every other mod's startup edits (FormList Manipulator INIs and the like),
  so their hand-made planter plants are kept and win.
- Merges the planter lists other mods ship in their plugins (for example Beyond Skyrim's
  BSKflPlanterPlantableItem / BSKflPlanterPlantedFlora), detected by shape, no EditorIDs needed.
- Picks the most planter-like plant when several give the same harvest
  (planter-made versions first, snowy variants last).
- Includes eggs from nests, hanging moss, honeycomb and harvested food crops.
- New ExcludeModelKeywords filter keeps out carcasses, bottles, wall trophies, bones,
  fish effects, mead barrels and skewers by default; editable in the INI.
- Counts runtime-added FormList entries, so the uneven-list safety check sees the
  true lengths and its message names the real cause.
- Log lists how many plants came from each mod.

## 0.2.0 — 2026-09-21 (merged)
- Merged the Qwen 0.1.0-mvp package and the Kimi DLL build into one project.
- Fixed the runtime version pin that would have prevented the plugin loading.
- Replaced the substring-based quest filter with a user blacklist.
- Scan now collects and validates before committing, so the two planter
  FormLists can no longer end up different lengths.
- Settings file is now actually read.
- Offline generator refuses to write empty or shrinking FormList overrides.
- Added the build files both earlier versions were missing (CMake presets,
  vcpkg manifest, PCH) and a CI workflow that compiles on GitHub.
- Removed both placeholder artifacts. No fake DLL, no empty ESL.

## 0.1.0-mvp — 2026-09-21 (Qwen, then Kimi)
- Scaffold and docs (Qwen); working scan logic and ESL writer (Kimi).
- Neither produced an installable mod.
