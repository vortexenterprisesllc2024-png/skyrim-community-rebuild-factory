# How we announce (GitHub + Nexus + community)

## 1. GitHub first (factory)
Create public repo e.g. `skyrim-community-rebuild-factory` under Jo’s GitHub (or a new org `skyrim-rebuilds`).
- Pin README
- Enable Releases for DLL zips
- Issues = rebuild requests
- Discussions = “who can take OIF this weekend?”

## 2. Nexus hub (players)
New mod page title (draft): **Community SKSE Rebuild Pack — AE 1.7.104**
- Category: Utilities / Modders Resources (or similar)
- Description: what broke, what we ship, install (Vortex after original mod), credits, GitHub link
- Files: versioned packs (`CRF-1.7.104-wave1.7z`) with per-DLL folders
- Soft requirements: SKSE 2.3.1, Address Library, original mods installed

## 3. Soft community ping (after Nexus is live)
- Nexus posts on original mod pages (polite: “unofficial 1.7 rebuild available here until author updates”)
- r/skyrimmods + Skyrim Modding Discord **only if Jo okays** each post
- Tag authors with thanks, not pressure

## 4. Cadence after every Bethesda / AL / SKSE bump
Day 0: open `queue/<version>.yaml`  
Day 0–2: triage hard-fail vs soft-skip  
Day 1–7: wave-1 essentials (BTPS-class, frameworks, po3 staples)  
Ongoing: accept PRs; retire entries when authors ship official builds
