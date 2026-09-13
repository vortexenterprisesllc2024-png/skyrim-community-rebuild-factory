# Rebuild recipe — Description Framework

| Field | Value |
|-------|--------|
| Queue id | `description-framework` |
| Nexus | [105799](https://www.nexusmods.com/skyrimspecialedition/mods/105799) |
| Upstream | https://github.com/Nightfallstorm/DescriptionFramework |
| License | **GPL-3.0** — private use OK; any redistributed binary **must** ship corresponding rebuild source |
| Base | Tag / release **2.1.2** (Nexus upload ~2025-08-30; pre-1.7). Repo version in CMakeLists = `2.1.2`. Prefer checkout of tag `2.1.2` if present; else commit that matches 2.1.2 release. |
| DLL out | `DescriptionFramework.dll` |
| Output dir | `C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\description-framework\` |
| Status intent | recipes / **rebuilding** — not claimed built |

**Why it fails on 1.7.104:** Submodule `extern/CommonLibSSE-NG` → `alandtse/CommonLibVR` @ **`738afc457fdeba050ce328f15225aa3cde674187`** — SSEv1/SSEv2/VR only, **no SSEv5**.

---

## 1. Clone (with submodules) and pin

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src
git clone --recurse-submodules https://github.com/Nightfallstorm/DescriptionFramework.git
cd DescriptionFramework
git fetch --tags
# Prefer release tag when available:
git checkout 2.1.2
# If tag missing, stay on the commit matching Nexus 2.1.2 / CMake VERSION 2.1.2
git submodule update --init --recursive
git -C extern\CommonLibSSE-NG rev-parse HEAD
# Expect old pin before bump: 738afc457fdeba050ce328f15225aa3cde674187
```

`.gitmodules` points at:

```text
extern/CommonLibSSE-NG  →  https://github.com/alandtse/CommonLibVR.git
```

---

## 2. Bump CommonLib submodule to SSEv5 `ng`

```powershell
cd extern\CommonLibSSE-NG
git fetch origin
git checkout ng
git pull --ff-only origin ng
$clib = git rev-parse HEAD
Write-Host "CommonLib ng pin: $clib"
Select-String -Path .\include\REL\IDDB.h -Pattern "SSEv5"
cd ..\..
# Record submodule change in parent (local rebuild branch — do not force-push upstream)
git add extern\CommonLibSSE-NG
git status
```

Pin requirement: commit must include `Format::SSEv5` and `load_v5` (tip was ~`d13d10a0…` / 8.0.1 on 2026-09-13 — **re-resolve** with `git ls-remote`).

---

## 3. Environment

Upstream README / CMakeLists use **`VCPKG_ROOT`**.  
`CMakePresets.json` cache var uses **`$env{VCPKG_INSTALLATION_ROOT}`**.

Set both to the same vcpkg root:

```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:VCPKG_INSTALLATION_ROOT = $env:VCPKG_ROOT
```

VS 2022 is required (preset generator `Visual Studio 17 2022`, toolset `v143`). Upstream README still mentions VS 2019 — use **2022**.

---

## 4. CMake configure + build

Preset name in `CMakePresets.json` is lowercase **`descriptionframework`** (README capitalizes `DescriptionFramework`; on Windows either usually works — prefer the file’s exact name).

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src\DescriptionFramework
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue

cmake --preset descriptionframework
cmake --build build --config Release
```

Triplet: `x64-windows-static`. Binary dir: `build\`.

### Expected artifact

```powershell
Get-ChildItem -Recurse -Filter DescriptionFramework.dll .\build
# typically: build\Release\DescriptionFramework.dll
```

---

## 5. Package to factory output (+ GPL source)

```powershell
$out = "C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\description-framework"
New-Item -ItemType Directory -Force -Path $out | Out-Null
Copy-Item -Force .\build\Release\DescriptionFramework.dll $out\
Copy-Item -Force .\LICENSE $out\

# GPL-3.0: keep rebuild source available next to the binary (zip of this working tree is fine)
$srcZip = Join-Path $out "DescriptionFramework-2.1.2-1.7.104-rebuild-src.zip"
# Example: compress without .git objects if desired; must include your CommonLib bump + sources
Compress-Archive -Path .\CMakeLists.txt,.\CMakePresets.json,.\cmake,.\src,.\include,.\extern,.\LICENSE,.\vcpkg.json -DestinationPath $srcZip -Force

Get-Item "$out\DescriptionFramework.dll" | Select-Object FullName, Length
Get-FileHash -Algorithm SHA256 "$out\DescriptionFramework.dll" | Format-List
```

Write `$out\BUILD-NOTES.md` with upstream ref, CommonLib submodule SHA, cmake commands, DLL size, SHA256, path to source zip, smoke result.

Keep Jo’s `*_DESC.ini` description packs from the original mod — replace **DLL only**.

---

## 6. Smoke tests

1. Drop new `DescriptionFramework.dll` over the parked/old one.
2. Boot SKSE 2.3.1 — **no** format-5 popup for Description Framework.
3. Log: `Documents\My Games\Skyrim Special Edition\SKSE\DescriptionFramework.log` (name may vary slightly — check SKSE folder).
4. Feature check: open inventory / enchanting UI and confirm description packs (`*_DESC`) still inject text.
5. UI menu hooks may need extra 1.7 validation — if text missing without REL errors, note as behavioral regression (do not claim full pass).

---

## 7. Notes / pitfalls

- Redistribution without matching source violates GPL-3.0 — always ship the rebuild source zip / link this factory PR.
- Prefer Nightfallstorm’s official 1.7.104 build if published; mark community rebuild superseded.
- Existing GitHub issues (#1–#3) are feature/UX, not format-5 — no need to wait on them for this rebuild.
