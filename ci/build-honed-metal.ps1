# Honed Metal SKSE plugin (Nexus 61015, author a_retarded_monkey) rebuilt for Skyrim AE 1.7.104 / SKSE 2.3.1.
# Classic skse64 plugin (NOT CommonLib): every game address it uses is a constant inside ianpatt/skse64's own
# headers/sources, so the plugin has to be compiled against the SKSE release that matches the game. A version-tag
# patch of an older build only fools the loader (Jo's 15 Sep test: CTD before the main menu).
# Source: Nexus 61015 file 466884 "Source Code" v7, plus a documented 1.26.1 script-compat patch
# (getItemsCount / pluginVersion 8) in sources/honed-metal/HonedMetal/. See sources/honed-metal/README.md.
# The author's ReadMe.txt recipe (Visual Studio: build skse64 as a Static Library + skse64_common, then the plugin)
# is reproduced here with a small CMake wrapper written at build time.
param(
    [string]$SkseRef = 'v2.3.1',                 # ianpatt/skse64 tag; its skse_version.h must say CURRENT_RELEASE_RUNTIME = 1.7.104
    [string]$CommonRef = 'master',               # ianpatt/common (SKSE's shared utility library); pinned sha goes into the notes
    [string]$OutDir = "$env:GITHUB_WORKSPACE\releases\1.7.104\honed-metal",
    [string]$CommonLibRef = '',                  # accepted for the workflow's uniform call; unused (no CommonLib in a classic plugin)
    [string]$VcpkgRef = ''                       # accepted for the workflow's uniform call; unused (no vcpkg needed)
)
. "$PSScriptRoot\common.ps1"

$root = "$env:RUNNER_TEMP\src"
$commonSrc = "$root\common"
$skseSrc = "$root\skse64"
$extern = "$root\extern"
$plugin = "$env:GITHUB_WORKSPACE\sources\honed-metal\HonedMetal"
$work = "$root\HonedMetal-1.7.104"
New-Item -ItemType Directory -Force -Path $root | Out-Null
if (-not (Test-Path "$plugin\src\main.cpp")) { throw "plugin source missing: $plugin\src\main.cpp" }

# ---- 1. ianpatt/common (static utility lib every skse64 target links) ---------------------------------------
if (-not (Test-Path "$commonSrc\.git")) { Invoke-Checked git clone --quiet https://github.com/ianpatt/common.git $commonSrc }
Push-Location $commonSrc
if ($CommonRef -ne 'master') { Invoke-Checked git checkout --quiet $CommonRef }
$commonSha = (git rev-parse HEAD).Trim()
Pop-Location
Write-Host "ianpatt/common = $commonSha"
Invoke-Checked cmake '-S' $commonSrc '-B' "$commonSrc\build" '-G' 'Visual Studio 17 2022' '-A' 'x64' "-DCMAKE_INSTALL_PREFIX=$extern"
Invoke-Checked cmake --build "$commonSrc\build" --config Release --target install

# ---- 2. ianpatt/skse64 at the tag that matches the game ---------------------------------------------------
if (-not (Test-Path "$skseSrc\.git")) { Invoke-Checked git clone --quiet --branch $SkseRef --depth 1 https://github.com/ianpatt/skse64.git $skseSrc }
Push-Location $skseSrc
$skseSha = (git rev-parse HEAD).Trim()
Pop-Location
Write-Host "ianpatt/skse64 $SkseRef = $skseSha"
$verH = Get-Content "$skseSrc\skse64_common\skse_version.h" -Raw
if ($verH -notmatch 'CURRENT_RELEASE_RUNTIME\s+RUNTIME_VERSION_1_7_104') { throw "skse64 $SkseRef does not target runtime 1.7.104 (CURRENT_RELEASE_RUNTIME)" }
if ($verH -notmatch 'SKSE_VERSION_VERSTRING\s+"0, 2, 3, 1"') { throw "skse64 $SkseRef is not SKSE 2.3.1" }

# ---- 3. plugin tree + CMake wrapper (author's ReadMe steps 3-7, minus the DLL's own entry point) -----------
Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $work | Out-Null
Copy-Item -Recurse -Force "$plugin\*" $work
$srcHashes = @()
foreach ($f in Get-ChildItem -Recurse -File $work) {
    $rel = $f.FullName.Substring($work.Length + 1)
    $srcHashes += "$((Get-FileHash -Algorithm SHA256 $f.FullName).Hash.ToLower())  $rel"
}
$skseCmake = ($skseSrc -replace '\\', '/')
@"
cmake_minimum_required(VERSION 3.18)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded`$<`$<CONFIG:Debug>:Debug>")
project(HonedMetal LANGUAGES CXX)
set(SKSE64_ROOT "$skseCmake" CACHE PATH "ianpatt/skse64 checkout")
include(`${SKSE64_ROOT}/cmake/versioning.cmake)
find_package(common REQUIRED CONFIG)
add_subdirectory(`${SKSE64_ROOT}/skse64_common skse64_common)
add_subdirectory(`${SKSE64_ROOT}/xbyak xbyak)
# skse64 as a static library = the author's ReadMe step 3. Same source list skse64's own CMake uses, minus skse64.cpp
# (the SKSE DLL's DllMain / loader glue, which a plugin must not carry).
include(`${SKSE64_ROOT}/skse64/cmake/sourcelist.cmake)
list(REMOVE_ITEM sources skse64.cpp)
list(TRANSFORM sources PREPEND "`${SKSE64_ROOT}/skse64/")
add_library(skse64_static STATIC `${sources})
target_compile_definitions(skse64_static PRIVATE RUNTIME RUNTIME_VERSION=`${RUNTIME_VERSION_PACKED})
target_compile_features(skse64_static PUBLIC cxx_std_11)
target_include_directories(skse64_static PUBLIC `${SKSE64_ROOT})
target_link_libraries(skse64_static PUBLIC skse64::skse64_common Xbyak::xbyak xse::common)
target_compile_options(skse64_static PRIVATE /MP /W3 /Oi)
# the plugin, as its own vcxproj builds it (Release|x64: DynamicLibrary, /MT, C++14, NDEBUG;_CONSOLE, includes = solution dir)
add_library(HonedMetal SHARED src/CraftingUtils.cpp src/IniParser.cpp src/main.cpp)
target_compile_features(HonedMetal PRIVATE cxx_std_14)
target_compile_definitions(HonedMetal PRIVATE NDEBUG _CONSOLE)
target_include_directories(HonedMetal PRIVATE `${SKSE64_ROOT})
target_link_libraries(HonedMetal PRIVATE skse64_static skse64::skse64_common xse::common)
target_compile_options(HonedMetal PRIVATE /MP /W3 /Oi)
target_link_options(HonedMetal PRIVATE /DEBUG:FULL /OPT:REF /OPT:ICF)
set_target_properties(HonedMetal PROPERTIES PREFIX "" OUTPUT_NAME "HonedMetal")
"@ | Out-File -Encoding ascii "$work\CMakeLists.txt"
Get-Content "$work\CMakeLists.txt"

# ---- 4. configure + build ---------------------------------------------------------------------------------
Invoke-Checked cmake '-S' $work '-B' "$work\build" '-G' 'Visual Studio 17 2022' '-A' 'x64' "-DCMAKE_PREFIX_PATH=$extern"
Invoke-Checked cmake --build "$work\build" --config Release --target HonedMetal
$dll = Get-ChildItem -Recurse -Filter HonedMetal.dll "$work\build" | Select-Object -First 1
if (-not $dll) { throw 'HonedMetal.dll not produced' }
Write-Host "built $($dll.FullName) ($($dll.Length) bytes)"

# ---- 5. verify the version block before anything ships (runtime 1.7.104.0 = 0x01070680, SKSE 2.3.1 layout) -
$verify = @'
import sys, struct, pefile
p = sys.argv[1]; data = open(p, 'rb').read(); pe = pefile.PE(data=data)
assert pe.FILE_HEADER.Machine == 0x8664, 'not x64'
exports = {e.name.decode(): e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
assert 'SKSEPlugin_Load' in exports and 'SKSEPlugin_Version' in exports, exports
off = pe.get_offset_from_rva(exports['SKSEPlugin_Version']); blk = data[off:off + 0x350]
dataVersion, pluginVersion = struct.unpack_from('<II', blk, 0)
name = blk[8:264].split(b'\0')[0].decode(); author = blk[264:520].split(b'\0')[0].decode()
indepEx, indep = struct.unpack_from('<II', blk, 772); compat = struct.unpack_from('<16I', blk, 780)
print(f'name={name!r} author={author!r} pluginVersion={pluginVersion} versionIndependenceEx={indepEx:#x} versionIndependence={indep:#x} compatibleVersions={[hex(v) for v in compat if v]}')
crt = [i.dll.decode().lower() for i in pe.DIRECTORY_ENTRY_IMPORT]
print('imports:', crt)
assert name == 'HonedMetal', name
assert pluginVersion == 8, 'pluginVersion must be 8 (Honed Metal 1.26.1)'
assert compat[0] == 0x01070680, 'compatibleVersions[0] is not 1.7.104.0'
assert indep == 0 and indepEx == 0, 'a classic SKSE plugin must not claim address-library independence'
assert not any(d.startswith(('vcruntime', 'msvcp', 'api-ms-win-crt')) for d in crt), 'expected /MT (static CRT)'
assert b'getItemsCount' in data, 'DLL must register getItemsCount (Honed Metal 1.26.1 / HonedMetal.bsa)'
assert b'getCachedItemCount' not in data, 'stale v7 getCachedItemCount must not remain (unbound GetItemsCount on 1.26.1 scripts)'
print('version block OK')
'@
$verify | Out-File -Encoding ascii "$work\verify_version_block.py"
Invoke-Checked python '-m' pip install --quiet pefile
Invoke-Checked python "$work\verify_version_block.py" $dll.FullName

# ---- 6. package -------------------------------------------------------------------------------------------
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Force $dll.FullName $OutDir
$pdb = $dll.FullName -replace '\.dll$', '.pdb'
if (Test-Path $pdb) { Copy-Item -Force $pdb $OutDir }
Copy-Item -Force "$work\ReadMe.txt" (Join-Path $OutDir 'ReadMe-upstream.txt') -ErrorAction SilentlyContinue
$len = (Get-Item (Join-Path $OutDir 'HonedMetal.dll')).Length
$sha = (Get-FileHash -Algorithm SHA256 (Join-Path $OutDir 'HonedMetal.dll')).Hash.ToLower()
"$sha  HonedMetal.dll" | Out-File -Encoding ascii (Join-Path $OutDir 'HonedMetal.dll.sha256')
New-SourceZip -SrcDir $work -ZipPath (Join-Path $OutDir 'HonedMetal-src-v7-1.7.104-rebuild-src.zip') -ExcludeDirNames @('build') -Readme @"
Honed Metal (Nexus 61015) SKSE plugin - source as rebuilt for Skyrim AE 1.7.104 / SKSE 2.3.1.
HonedMetal source: Nexus 61015 file 466884 "Source Code" v7 by a_retarded_monkey, plus the 1.26.1 getItemsCount / pluginVersion 8 compat patch (see BUILD-NOTES.md).
CMakeLists.txt + verify_version_block.py: the factory's wrapper (replaces the author's Visual Studio steps; see ReadMe.txt).
Built against: https://github.com/ianpatt/skse64 tag $SkseRef (commit $skseSha) as a static library (skse64.cpp excluded)
               https://github.com/ianpatt/common commit $commonSha (installed with cmake, prefix extern/)
Build: cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=<extern> -DSKSE64_ROOT=<skse64 checkout>
       cmake --build build --config Release --target HonedMetal      (VS 2022 v143, /MT)
"@
$notes = @()
$notes += "# BUILD-NOTES - Honed Metal (SKSE plugin) - Skyrim AE 1.7.104.0 / SKSE 2.3.1"
$notes += ""
$notes += "Community rebuild from the author's published source. Not an official author build. Built on a GitHub Actions ``windows-2022``"
$notes += "runner by the skyrim-community-rebuild-factory workflow (run $env:GITHUB_RUN_ID, $(Get-Date -Format 'yyyy-MM-dd HH:mm') UTC)."
$notes += "This replaces the 15 Sep 2026 version-tag hex patch of the 1.6.1170 DLL, which SKSE loaded and which crashed before the main menu."
$notes += ""
$notes += "| Field | Value |"
$notes += "|---|---|"
$notes += "| Upstream | Nexus 61015 (Honed Metal - NPC Crafting and Enchanting Services), author a_retarded_monkey |"
$notes += "| Source used | Nexus 61015 file 466884 ``Source Code`` v7 plus the 1.26.1 script-compat patch (``getItemsCount`` / ``pluginVersion`` 8). Per-file SHA-256 below. |"
$notes += "| Upstream licence | per the Nexus 61015 page / author (source ships with derivatives - this zip and the repo copy) |"
$notes += "| Build model | classic skse64 plugin: ianpatt/skse64 compiled as a static library + skse64_common + ianpatt/common (author's ReadMe.txt), NOT CommonLibSSE |"
$notes += "| skse64 | https://github.com/ianpatt/skse64 tag ``$SkseRef`` (commit ``$skseSha``): CURRENT_RELEASE_RUNTIME = RUNTIME_VERSION_1_7_104, SKSE 2.3.1 |"
$notes += "| common | https://github.com/ianpatt/common commit ``$commonSha`` |"
$notes += "| Configure | ``cmake -S . -B build -G `"Visual Studio 17 2022`" -A x64 -DCMAKE_PREFIX_PATH=<extern> -DSKSE64_ROOT=<skse64>`` (wrapper CMakeLists.txt in the src zip) |"
$notes += "| Build | ``cmake --build build --config Release --target HonedMetal`` (v143, /MT, C++14, NDEBUG;_CONSOLE as in HonedMetal.vcxproj Release x64) |"
$notes += "| DLL | ``HonedMetal.dll`` |"
$notes += "| DLL size | $len bytes |"
$notes += "| DLL SHA256 | ``$sha`` |"
$notes += "| Version block | pluginVersion = 8 (1.26.1), compatibleVersions[0] = 1.7.104.0 (0x01070680), no address-library-independence flags - correct for a classic plugin, which must be rebuilt for each game version (verified by verify_version_block.py in the workflow) |"
$notes += "| Compiler | MSVC (Visual Studio 2022, v143) on windows-2022; see the workflow log for the exact ``cl.exe`` version |"
$notes += "| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to the main menu with music, see ``HonedMetal.dll (...) loaded correctly`` in skse64.log. Papyrus.0.log must not show unbound GetItemsCount / failed getCachedItemCount bind. Then run Farengar's (or Wuunferth's) enchant service. |"
$notes += ""
$notes += "## Source files (SHA-256 of the tree that was compiled)"
$notes += ""
foreach ($h in $srcHashes) { $notes += "- ``$h``" }
$notes += ""
$notes += "## Source changes made for this rebuild"
$notes += ""
$notes += "- Compatibility patch vs Nexus 61015 file 466884: replace ``getCachedItemCount(REFR*, Form*) -> SInt32`` with ``getItemsCount(REFR*, Form[]) -> Int[]`` so the native matches Honed Metal 1.26.1 ``HonedMetal.bsa`` ``HMCraftingUtils``. Extra-change deltas stay cached per container; TESContainer base counts are added per requested form."
$notes += "- ``pluginVersion`` set to 8 (1.26.1 FOMOD identity). Published v7 source said 7; the author published no later source."
$notes += "- The wrapper CMakeLists.txt reproduces HonedMetal.vcxproj's Release|x64 settings and the ReadMe's static-library recipe; skse64.cpp (the SKSE DLL's own DllMain and loader glue) is left out of the static library, as a plugin must not carry it."
$notes += "- TARGET_RUNTIME resolves to CURRENT_RELEASE_RUNTIME from the 2.3.1 headers (SKSE_resolver.h), so the runtime check in validate_plugin() and compatibleVersions[0] are both 1.7.104.0."
$notes += ""
$notes += "## Caveats"
$notes += ""
$notes += "- Struct layouts (InventoryEntryData, ExtraContainerChanges, DataHandler::arrPERK, EquipManager, MenuManager) come from SKSE 2.3.1's headers for 1.7.104. If SKSE's own headers lag the game for any of these, the symptom is wrong item stats or a crash inside Honed Metal's natives, not a load failure."
$notes += "- Eight Papyrus natives and one Scaleform inventory callback; the first real exercise is the Farengar (or any Honed Metal NPC) enchant/temper service. Test with a save you can throw away."
$notes += "- Keep HonedMetal.esp enabled; replace only HonedMetal.dll (Data and the Honed Metal 1.26 Vortex staging copy). Do not FOMOD-reinstall."
$notes += "- If the author publishes a 1.7.104 build, it supersedes this one."
$notes -join "`n" | Out-File -Encoding utf8 (Join-Path $OutDir 'BUILD-NOTES.md')
Write-Host "wrote BUILD-NOTES.md (HonedMetal.dll $len bytes, sha256 $sha)"
Get-ChildItem $OutDir | Format-Table Name, Length

# robocopy (inside New-SourceZip) exits 1 when it copied files; the runner would otherwise report that as the step result
$global:LASTEXITCODE = 0
