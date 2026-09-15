# sources/honed-metal

Upstream source of the **Honed Metal** SKSE plugin (Nexus Mods 61015, *Honed Metal - NPC Crafting and Enchanting Services*, author a_retarded_monkey), kept here so the factory can rebuild `HonedMetal.dll` for Skyrim AE 1.7.104 / SKSE 2.3.1 (`ci/build-honed-metal.ps1`).

- `HonedMetal/` starts from Nexus 61015 file **466884** "Source Code" v7 (the author's published source zip). `HonedMetal.vcxproj.user` (168 bytes of empty project state) is the only file left out of that zip.
- It is a classic skse64 plugin: it compiles against ianpatt/skse64 built as a static library (see `HonedMetal/ReadMe.txt`). The factory does that with a CMake wrapper generated at build time.
- Licence / permissions: per the Nexus 61015 page and the author (the author asks that source ship with derivatives; the rebuilt DLL ships with this tree and the build notes). If the author objects to this copy, it comes down.
- **1.26.1 script compatibility patch** (this tree is no longer byte-identical to file 466884):
  - Honed Metal **1.26.1** ships `HonedMetal.bsa` `HMCraftingUtils` as  
    `Int[] Function getItemsCount(ObjectReference container, Form[] items) native global`.
  - File 466884 still registers `SInt32 getCachedItemCount(TESObjectREFR*, TESForm*)`, which Papyrus cannot bind to those scripts (`Unbound native function "GetItemsCount"` / failed `getCachedItemCount` bind). The 1.26.1 AE_1.6.1170 DLL strings contain `getItemsCount`; the unmodified v7 rebuild did not.
  - `CraftingUtils.cpp` now registers `getItemsCount` returning `VMResultArray<SInt32>` with args `TESObjectREFR*` + `VMArray<TESForm*>`. Extra-container deltas stay cached per container (v7 idea); TESContainer base counts are added per requested form so NPC stock items are counted.
  - `main.cpp` `pluginVersion` is **8** (what the 1.26.1 FOMOD DLL reports). v7 source said 7; the author published no later source.

Remove this folder if the author publishes a 1.7.104 build - that supersedes the rebuild.
