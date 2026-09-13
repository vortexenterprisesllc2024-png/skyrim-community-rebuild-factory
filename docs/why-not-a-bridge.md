# Why not a universal Address Library “bridge”?

Players ask: can one shim make every pre-1.7.99 DLL load on format 5?

**Short answer: not safely.**

Each SKSE plugin embeds its own CommonLib/REL reader. That code hard-checks the `.bin` format. Soft-skips (“must be recompiled…”) happen inside SKSE before the plugin runs. Force-loading mismatched plugins is how you get silent memory corruption and save damage.

A **narrow** binary patch that only flips the format gate can help a *subset* of hard-fail DLLs as a triage experiment — but it is not a community product. The durable fix is **rebuild against SSEv5 CommonLib**, which is what this factory automates.

Related: SE 1.5 → AE 1.6 already proved Address Library ID spaces can break across major jumps; format gates exist for a reason.
