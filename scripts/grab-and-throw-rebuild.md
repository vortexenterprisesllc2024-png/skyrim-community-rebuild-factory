# Rebuild recipe — Grab And Throw (powerof3)

| Field | Value |
|-------|--------|
| Queue id | `grab-and-throw` |
| Nexus | [120460](https://www.nexusmods.com/skyrimspecialedition/mods/120460) |
| Upstream | https://github.com/powerof3/GrabAndThrow |
| License | **MIT** |
| Base tag | **`v2.1.2`** (required). Jo previously had 2.1.1; GitHub `v2.1.2` is still pre-1.7.99 / format-5 era. |
| DLL out | `po3_GrabAndThrow.dll` |
| Output dir | `C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\grab-and-throw\` |
| Status intent | recipes / **rebuilding** — not claimed built |

**Why it fails on Jo’s 1.7.104 boot:** `po3_GrabAndThrow.dll` hard-popups (same Address Library format-5 / old CommonLib class as other wave-1 plugins). Stock CommonLib wiring is **powerof3/CommonLibSSE** (`extern/CommonLibSSE` or `CommonLibSSEPath`) — must be replaced or redirected to an **SSEv5-capable** tree (`alandtse/CommonLibVR` `ng` or equivalent).

OIF’s `"Throw"` JSON event soft-depends on this mod — rebuild Grab And Throw so OIF throw rules work after OIF itself is restored.

---

## 1. Clone and checkout `v2.1.2`

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src
git clone --recurse-submodules https://github.com/powerof3/GrabAndThrow.git
cd GrabAndThrow
git fetch --tags
git checkout v2.1.2
git submodule update --init --recursive
git rev-parse HEAD
# Tag object tip was 7f5445c9b7859072a369a4310cfcaa9305fb3bc4 — confirm locally
```

Upstream README build (AE):

```text
cmake --preset vs2022-windows-vcpkg-ae
cmake --build buildae --config Release
```

---

## 2. Point CommonLib at SSEv5 (`ng`)

`CMakeLists.txt` resolves CommonLib as:

1. `extern/CommonLibSSE` (submodule → `powerof3/CommonLibSSE`), else
2. Env var **`CommonLibSSEPath`** (directory containing `include/REL/Relocation.h`).

AE builds set `BUILD_SKYRIMAE` / `SKYRIM_AE` / `SKYRIM_SUPPORT_AE`.

### Recommended approach (cleanest for 1.7.104)

Clone CommonLibVR `ng` beside the tree and point the env var at it (avoids fighting the old powerof3 submodule):

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src
git clone https://github.com/alandtse/CommonLibVR.git
cd CommonLibVR
git checkout ng
git pull --ff-only origin ng
$clib = git rev-parse HEAD
Write-Host "CommonLib ng pin: $clib"
Select-String -Path .\include\REL\IDDB.h -Pattern "SSEv5"

# For the GrabAndThrow shell session:
$env:CommonLibSSEPath = "C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src\CommonLibVR"
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
```

### Alternate approach

Replace submodule contents:

```powershell
cd GrabAndThrow\extern\CommonLibSSE
git remote add ng https://github.com/alandtse/CommonLibVR.git   # if needed
git fetch ng
git checkout ng/<FULL_SHA>   # or reset hard to ng tip
```

Only use this if you understand submodule surgery; prefer `CommonLibSSEPath`.

**Do not** build against an untouched powerof3/CommonLibSSE `dev` pin unless you have verified it already contains `Format::SSEv5` / 1.7.104 Address Library support.

---

## 3. CMake configure + build (AE)

```powershell
cd C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\src\GrabAndThrow
# Ensure CommonLibSSEPath + VCPKG_ROOT still set in this shell
Remove-Item -Recurse -Force .\buildae -ErrorAction SilentlyContinue

cmake --preset vs2022-windows-vcpkg-ae
cmake --build buildae --config Release
```

Notes from upstream presets:

- Binary dir: **`buildae`**
- Triplet: `x64-windows-static`
- Overlay ports: `cmake/ports/` (clib-util only — CommonLib is **not** a vcpkg dep here)
- `COPY_BUILD` option may try to copy into `SkyrimAEPath` if set — fine if configured; otherwise ignore copy failures after a successful link.

### Expected artifact

```powershell
Get-ChildItem -Recurse -Filter po3_GrabAndThrow.dll .\buildae
# typically: buildae\Release\po3_GrabAndThrow.dll
```

---

## 4. Package to factory output

```powershell
$out = "C:\Users\hoefl\Downloads\skyrim-work\community-rebuild-factory\1.7.104\grab-and-throw"
New-Item -ItemType Directory -Force -Path $out | Out-Null
Copy-Item -Force .\buildae\Release\po3_GrabAndThrow.dll $out\
Copy-Item -Force .\LICENSE $out\

Get-Item "$out\po3_GrabAndThrow.dll" | Select-Object FullName, Length
Get-FileHash -Algorithm SHA256 "$out\po3_GrabAndThrow.dll" | Format-List
```

Write `$out\BUILD-NOTES.md` with: tag `v2.1.2`, CommonLib path + SHA, cmake commands, DLL size, SHA256, smoke result.

Keep Jo’s existing `po3_GrabAndThrow.ini` (and any MCM/settings) — replace **DLL only**.

---

## 5. Smoke tests

1. Drop new `po3_GrabAndThrow.dll` over the old hard-popup DLL.
2. Boot SKSE 2.3.1 — **no** format-5 / hard popup for Grab And Throw.
3. Log: `Documents\My Games\Skyrim Special Edition\SKSE\po3_GrabAndThrow.log` (confirm exact name in SKSE folder).
4. Feature check:
   - Hold **E** to grab a world object.
   - **R** to throw.
   - Confirm ini / MCM settings still apply (`po3_GrabAndThrow.ini`).
5. After OIF rebuild is also installed: verify an OIF JSON rule with `"event": ["Throw"]` still triggers.

---

## 6. Notes / pitfalls

- If configure says CommonLib path not found, `CommonLibSSEPath` was not visible to that cmake process — set it in the **same** shell before `cmake --preset`.
- API/layout drift between powerof3 CommonLibSSE and alandtse NG may cause compile errors — fix forward (includes, namespaces) rather than falling back to a non-SSEv5 pin.
- Prefer an official powerof3 1.7.104 / post-format-5 release when it lands; mark this rebuild superseded.
