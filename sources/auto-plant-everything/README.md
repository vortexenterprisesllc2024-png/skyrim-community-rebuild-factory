# Auto Plant Everything 0.2.0

Adds modded plants to Hearthfire / Goldenhills planters, so anything that drops
an ingredient when harvested can be grown in a planter.

Jo: read `START-HERE.txt`, not this file.

- `START-HERE.txt` — plain-language guide and build steps
- `AI-HANDOFF.md` — technical state, what was fixed, what's unverified
- `src/` — the SKSE plugin (never compiled yet)
- `tools/generate_esl.py` — offline generator, for games that can't load SKSE plugins
- `package/` — the settings file that ships alongside the built DLL
- `.github/workflows/build.yml` — builds the DLL on GitHub's Windows runners

Status: source complete, never compiled, not installed anywhere. The working
solution on Jo's machine today is `Grokbot-AllPlantablePlants.esp` (185 pairs).
