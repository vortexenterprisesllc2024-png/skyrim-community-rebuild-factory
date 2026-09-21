# Auto Plant Everything 0.2.0

SKSE plugin that appends missing harvestable plants to the Hearthfire planter
FormLists (`flPlanterPlantableItem` / `flPlanterPlantedFlora`) at `kDataLoaded`.

This is original plugin source, built by the community rebuild factory for
Skyrim AE 1.7.104 / Address Library format 5. It is not a drop-in replacement
for `Grokbot-AllPlantablePlants.esp`. That plugin stays the live planter fix
until this DLL has been smoked in-game.

- `src/` — plugin (NG 8.x: `UsesAddressLibrary()`, `UsesNoStructs()`)
- `package/SKSE/Plugins/AutoPlantEverything.ini` — settings shipped next to the DLL
- `tools/generate_esl.py` — offline ESL generator; refuses empty or shrinking FormList overrides
- `cmake/ports/commonlibsse-ng/` — vcpkg overlay template; the build script pins CommonLib

Build: `ci/build-autoplanteverything.ps1` on the factory's Windows runner.
The install zip is `AutoPlantEverything-0.2.0.zip` (`SKSE/Plugins/` DLL + INI, no FOMOD).
