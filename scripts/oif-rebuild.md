# Rebuild recipe — Object Impact Framework (OIF)

| Field | Value |
|-------|--------|
| Queue id | `oif` |
| Nexus | [149484](https://www.nexusmods.com/skyrimspecialedition/mods/149484) |
| Upstream | https://github.com/arachnocid/ObjectImpactFramework |
| License | **Apache-2.0** |
| Base tag | **1.6.8** (2025-08-28) — last public release; no official 1.7.104 build found |
| DLL out | `ObjectImpactFramework.dll` |
| Output dir | `C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\oif\` |
| Status intent | recipes / **rebuilding** — not claimed built |

**Why it fails on 1.7.104:** Shipping build links CommonLib pin **`e617713…`** (SSEv1/SSEv2/VR only). Address Library for 1.7.104 is **format 5**. Renaming bins will not fix this.

**CIF does not replace OIF.** Do not skip this rebuild because Core Impact Framework is installed.

---

## 1. Clone and pin tag

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src
git clone https://github.com/arachnocid/ObjectImpactFramework.git
cd ObjectImpactFramework
git fetch --tags
git checkout 1.6.8
git status
git rev-parse HEAD
```

Expected tag tip (verify): tag `1.6.8` → commit `ec2b126b5fb0ae98311ceb30fc8acdf28d8936a2` (confirm with `git rev-parse 1.6.8^{}`).

---

## 2. Bump CommonLib (critical)

OIF pulls CommonLib via a **vcpkg overlay port**:

```text
cmake\ports\commonlibsse-ng\portfile.cmake
```

Stock pin at tag 1.6.8:

```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO alandtse/CommonLibVR
    REF e617713b2ae8a927bf925d1ad138cc48ab72e414   # NO SSEv5
    SHA512 997238522e1433dd81c73b4762ae8ec0e919ac65604570924ae5a76ba7d4428dfe8d961cf1fdc693744dba6be66bf45ba16d6a603ca9bb8234f08aedf2df146d
    HEAD_REF ng
)
```

### 2a. Choose SSEv5-capable REF

Use current `alandtse/CommonLibVR` **`ng`** tip that contains `Format::SSEv5` / `load_v5` (CommonLibSSE-NG ≥ ~6.6–6.7; tip was **8.0.1** / `d13d10a0…` on 2026-09-13 — **re-resolve**):

```powershell
git ls-remote https://github.com/alandtse/CommonLibVR.git refs/heads/ng
# record FULL_SHA
```

Optionally clone once to verify:

```powershell
git clone --depth 1 -b ng https://github.com/alandtse/CommonLibVR.git "$env:TEMP\CommonLibVR-ng-check"
Select-String -Path "$env:TEMP\CommonLibVR-ng-check\include\REL\IDDB.h" -Pattern "SSEv5"
```

### 2b. Edit `portfile.cmake`

1. Set `REF` to the full `ng` SHA (40 hex chars).
2. Replace `SHA512` with a placeholder empty / wrong value so the first configure prints the expected hash, **or** compute it:

```powershell
# Example: download GitHub archive and hash (PowerShell)
$sha = "<FULL_SHA>"
$url = "https://github.com/alandtse/CommonLibVR/archive/$sha.tar.gz"
$out = "$env:TEMP\CommonLibVR-$sha.tar.gz"
Invoke-WebRequest -Uri $url -OutFile $out
# vcpkg expects SHA512 of the archive bytes:
(Get-FileHash -Algorithm SHA512 $out).Hash.ToLower()
```

Paste that SHA512 into `portfile.cmake`.

3. Keep `REPO alandtse/CommonLibVR` and `HEAD_REF ng`.
4. Optionally bump `cmake\ports\commonlibsse-ng\vcpkg.json` `version-semver` to match the CommonLib release (e.g. `8.0.1`) for clarity — not required for linking if REF is correct.

### 2c. Clear stale vcpkg packages (recommended)

```powershell
# From a clean shell with VCPKG_ROOT set
& "$env:VCPKG_ROOT\vcpkg.exe" remove commonlibsse-ng:x64-windows-static-md --recurse
# Or delete the build tree so the overlay port is re-fetched:
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue
```

---

## 3. CMake configure + build (AE Release)

Env:

```powershell
# Required by OIF CMakePresets (VCPKG_ROOT)
$env:VCPKG_ROOT = "C:\path\to\vcpkg"   # if not already a user env var
```

Presets (from upstream `CMakePresets.json`): configure/build preset name **`AE`**.

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src\ObjectImpactFramework

cmake --preset AE
cmake --build --preset AE
# equivalent:
# cmake --build build --config Release
```

Triplet used by OIF: `x64-windows-static-md`. Generator: Visual Studio 17 2022.

### Expected artifact

Look under:

```text
build\Release\ObjectImpactFramework.dll
# or build\<config>\ depending on generator layout — confirm with:
Get-ChildItem -Recurse -Filter ObjectImpactFramework.dll .\build
```

---

## 4. Package to factory output

```powershell
$out = "C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\oif"
New-Item -ItemType Directory -Force -Path $out | Out-Null
Copy-Item -Force .\build\Release\ObjectImpactFramework.dll $out\
Copy-Item -Force .\LICENSE $out\   # Apache-2.0

Get-Item "$out\ObjectImpactFramework.dll" | Select-Object FullName, Length
Get-FileHash -Algorithm SHA256 "$out\ObjectImpactFramework.dll" | Format-List
```

Write `$out\BUILD-NOTES.md` with: upstream tag `1.6.8`, CommonLib REF SHA, cmake commands, DLL size, SHA256, smoke result.

**Do not** replace Jo’s `Data\SKSE\Plugins\ObjectImpactFramework\*.json` rule packs — only the DLL.

---

## 5. Smoke tests

1. Backup old `ObjectImpactFramework.dll`; drop the new one into the Vortex/MO2 mod.
2. Ensure `versionlib-1-7-104-0.bin` remains installed.
3. Boot via SKSE 2.3.1.
4. **Pass criteria:**
   - No format-5 / “failed to open address library” popup for OIF.
   - `Documents\My Games\Skyrim Special Edition\SKSE\ObjectImpactFramework.log` opens without REL format errors.
   - With existing OIF JSON packs enabled: **Activate**, **Grab**, **Throw** (Throw needs Grab And Throw), and a simple **Hit** rule still fire.
5. If load succeeds but an event crashes → treat as **hook/RelocationID drift** (1.6.1170 → 1.7.104). Log the fault; do not ship silently. Same class of work as Fleikon BTPS / community 1.7 patches.

---

## 6. Notes / pitfalls

- First vcpkg configure after REF bump is slow (rebuilds CommonLib).
- If SHA512 mismatches, cmake/vcpkg error text includes the expected hash — paste it and re-run.
- Prefer an official arachnocid 1.7 drop if one appears; mark this rebuild superseded.
- Sibling **CIF** (Nexus 146873) is unrelated combat-impact tech — leave it alone for this recipe.
