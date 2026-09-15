# sources/honed-metal

Upstream source of the **Honed Metal** SKSE plugin (Nexus Mods 61015, *Honed Metal - NPC Crafting and Enchanting Services*, author a_retarded_monkey), kept here so the factory can rebuild `HonedMetal.dll` for Skyrim AE 1.7.104 / SKSE 2.3.1 (`ci/build-honed-metal.ps1`).

- `HonedMetal/` = Nexus 61015 file **466884** "Source Code" v7 (the author's published source zip), **unmodified**, byte-identical to the download (SHA-256 per file in every `releases/1.7.104/honed-metal/BUILD-NOTES.md`). `HonedMetal.vcxproj.user` (168 bytes of empty project state) is the only file left out.
- It is a classic skse64 plugin: it compiles against ianpatt/skse64 built as a static library (see `HonedMetal/ReadMe.txt`). The factory does that with a CMake wrapper generated at build time; nothing here is patched.
- Licence / permissions: per the Nexus 61015 page and the author (the author asks that source ship with derivatives; the rebuilt DLL ships with this tree and the build notes). If the author objects to this copy, it comes down.
- The 1.26.1 FOMOD's DLL reports plugin version 8; this source says 7. v7 is the latest source the author published.

Remove this folder if the author publishes a 1.7.104 build - that supersedes the rebuild.
