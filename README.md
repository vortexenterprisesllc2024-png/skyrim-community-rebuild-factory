# Skyrim Community Rebuild Factory

**Goal:** After each Skyrim / Address Library / SKSE jump, cut DLL downtime from weeks–months to **days** for the most-used SKSE plugins — without waiting on every author.

**Status:** Seed project (2026-09-13). Not yet public. First target runtime: **Skyrim AE 1.7.104.0** + **SKSE 2.3.1** + Address Library **format 5** (`versionlib-1-7-104-0.bin`).

---

## How this works (two doors)

| Door | What it is | Who uses it |
|------|------------|-------------|
| **GitHub (this repo)** | Factory: source notes, rebuild recipes, CI, release assets, contributor queue | Modders / rebuilders |
| **Nexus hub page** | Player front door: Vortex-friendly DLL pack + credits + link here | Players |

Players should not need GitHub. Contributors should not need Nexus to submit a rebuild PR.

---

## What we rebuild (and what we don’t)

**We do**
- Open-source SKSE plugins that hard-fail on Address Library **format 5** (old CommonLib only knows formats 1/2)
- Documented, licensed rebuilds (Apache-2.0, MIT, GPL with compliance)
- Drop-in replacement DLLs that keep original JSON/INI/content packs working
- Credit original authors first; we are a **compatibility bridge**, not a fork takeover

**We don’t**
- Claim official authorship
- Redistribute closed-source binaries we can’t rebuild
- Ship a “magic bridge” that force-loads arbitrary old DLLs (unsafe; see `docs/why-not-a-bridge.md`)
- Nag authors — we open issues politely and prefer their official build when it lands

---

## Priority queue (1.7.104)

See `queue/1.7.104.yaml`. First wave:

1. **Object Impact Framework** (arachnocid) — Apache-2.0 — essential object interaction JSON
2. **Description Framework** (Nightfallstorm) — GPL-3.0 — description packs
3. **Grab And Throw** (powerof3) — MIT — hard-popup on Jo’s list
4. Catalog existing community patches (**Fleikon BTPS**, etc.) so players get one pack

---

## Root cause in one sentence

Address Library for 1.7.x is fine; many pre-1.7 DLLs were built against CommonLib that **cannot read format 5**. Fix = rebuild against CommonLib with **SSEv5** (`alandtse/CommonLibVR` `ng` / CommonLibSSE-NG ≥ ~6.6–6.7).

---

## Contributing

See `docs/CONTRIBUTING.md`. Short version: pick a queued mod → bump CommonLib → build AE → smoke-test → PR with DLL + notes.

## License

- Repo docs/scripts: MIT (unless noted)
- Format-5 rebuilds link CommonLibSSE-NG (**GPL-3.0-or-later** + Modding Exception). Redistribution of those DLLs is a GPL-combined work and **must** include corresponding source (`*-rebuild-src.zip` in each release folder). The upstream plugin licence still applies to plugin code.
- Always keep upstream LICENSE in the release folder

## Credits

Original plugin authors · SKSE · meh321 (Address Library) · CommonLib maintainers · Fleikon/Juan-MZ (BTPS 1.7.99 pattern) · Jo Hoeflich (seed + player pack)
