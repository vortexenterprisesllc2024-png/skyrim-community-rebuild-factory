# DRAFT — Nexus page (do not publish until Jo says)

**Title:** Community SKSE Rebuild Pack — AE 1.7.104

**Short description:**
Unofficial drop-in SKSE DLL rebuilds for Skyrim AE 1.7.104 (Address Library format 5). Cuts wait time after game updates. Not affiliated with original authors — we rebuild open-source plugins so your load order keeps working.

**Requirements:**
- Skyrim AE 1.7.104
- SKSE 2.3.1
- Address Library for SKSE Plugins (current All-in-one with versionlib-1-7-104-0.bin)
- The *original* mods installed (we only replace DLLs)

**Install (Vortex):**
1. Install/enable the original mod as usual.
2. Install this pack *after* it (or drop the rebuilt DLL over Data/SKSE/Plugins).
3. Deploy. Launch via SKSE.

**What this is for:**
After 1.7.x, many SKSE DLLs fail with “Unsupported address library format: 5” because they were built on an older CommonLib. Reinstalling Address Library does not fix that — the DLL must be rebuilt. This pack collects those rebuilds.

**Wave 1 (planned):**
- Object Impact Framework (rebuild)
- Description Framework (rebuild)
- Grab And Throw (rebuild)
- Better Third Person Selection — Fleikon 1.7.99 community patch (catalogued)

**Credits:** Original authors always first. CommonLib / SKSE / Address Library maintainers. Community patch authors (e.g. Fleikon / Juan-MZ for BTPS). Factory maintained with help from Jo Hoeflich + contributors.

**Source / contribute:** [GitHub repo URL TBD]

**Permissions:** Format-5 rebuilds link CommonLibSSE-NG (GPL-3.0-or-later + Modding Exception), so redistribution must include corresponding source (`*-rebuild-src.zip` in each release folder). The upstream plugin licence still applies to plugin code. Authors: if you release an official 1.7 build, tell us — we mark ours superseded and point to you.
