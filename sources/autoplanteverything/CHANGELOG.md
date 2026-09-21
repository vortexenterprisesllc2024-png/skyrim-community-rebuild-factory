# Changelog

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
