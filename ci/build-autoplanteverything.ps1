# Auto Plant Everything 0.2.0 (Jo Hoeflich) built for Skyrim AE 1.7.104 / Address Library format 5.
# Recipe: scripts/autoplanteverything-rebuild.md.
# Source lives in this repo (sources/autoplanteverything). CommonLib comes in through the vcpkg overlay
# port cmake/ports/commonlibsse-ng, pinned the same way as ci/build-oif.ps1: alandtse/CommonLibVR ng
# commit that contains Format::SSEv5. The sources are already NG 8.x (UsesAddressLibrary() with no
# argument, UsesNoStructs()). This script does not patch those calls.
param(
    [string]$CommonLibRef = 'd13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd',
    [string]$VcpkgRef = 'a1cae005c39be7b18ba319fced856b68d7276271',
    [string]$OutDir = "$env:GITHUB_WORKSPACE\releases\1.7.104\autoplanteverything"
)
. "$PSScriptRoot\common.ps1"

$origin = "$env:GITHUB_WORKSPACE\sources\autoplanteverything"
if (-not (Test-Path "$origin\src\main.cpp")) { throw "plugin source missing: $origin\src\main.cpp" }
if (-not (Test-Path "$origin\tools\generate_esl.py")) { throw "generate_esl.py missing" }
if (-not (Test-Path "$origin\package\SKSE\Plugins\AutoPlantEverything.ini")) { throw "INI missing" }

# ---- 0. empty-override refusal (the generator must not ship a zero-entry FLST) ----
$gen = Join-Path $origin 'tools\generate_esl.py'
$safetyRoot = Join-Path $env:RUNNER_TEMP 'ape-safety'
New-Item -ItemType Directory -Force -Path $safetyRoot | Out-Null
$noData = Join-Path $safetyRoot 'no-data.esl'
& python $gen --data (Join-Path $safetyRoot 'missing-data') --plugins (Join-Path $safetyRoot 'missing.txt') --out $noData
if ($LASTEXITCODE -eq 0) { throw 'SAFETY TEST FAILED: generator wrote a file with no Data folder' }
if (Test-Path $noData) { throw 'SAFETY TEST FAILED: generator leaked a file with no Data folder' }
$fakeData = Join-Path $safetyRoot 'data'
New-Item -ItemType Directory -Force -Path $fakeData | Out-Null
New-Item -ItemType File -Force -Path (Join-Path $fakeData 'Skyrim.esm') | Out-Null
Set-Content -Encoding ascii -Path (Join-Path $fakeData 'plugins.txt') -Value '*Skyrim.esm'
$noLists = Join-Path $safetyRoot 'no-lists.esl'
& python $gen --data $fakeData --plugins (Join-Path $fakeData 'plugins.txt') --out $noLists
if ($LASTEXITCODE -eq 0) { throw 'SAFETY TEST FAILED: generator wrote a file with no Hearthfire lists' }
if (Test-Path $noLists) { throw 'SAFETY TEST FAILED: generator leaked a file with no Hearthfire lists' }
Write-Host 'generator safety checks passed (no empty FormList override)'

# ---- 1. work tree (patches stay off the checkout) ---------------------------
$src = "$env:RUNNER_TEMP\src\AutoPlantEverything"
if (Test-Path $src) { Remove-Item -Recurse -Force $src }
New-Item -ItemType Directory -Force -Path $src | Out-Null
& robocopy $origin $src /E /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -ge 8) { throw "robocopy of sources failed with $LASTEXITCODE" }
$global:LASTEXITCODE = 0

$vcpkgHead = Initialize-Vcpkg -Ref $VcpkgRef
Push-Location $src

# Point the manifest baseline at the vcpkg commit we actually cloned, so fmt/spdlog
# resolve to versions NG 8.0.1 can compile against.
$manifestPath = Join-Path $src 'vcpkg.json'
$manifestText = [IO.File]::ReadAllText($manifestPath)
$manifestUpdated = [regex]::Replace($manifestText, '"builtin-baseline"\s*:\s*"[^"]*"', ('"builtin-baseline": "' + $vcpkgHead + '"'))
if ($manifestUpdated -eq $manifestText) { throw 'vcpkg.json builtin-baseline was not updated' }
[IO.File]::WriteAllText($manifestPath, $manifestUpdated, (New-Object System.Text.UTF8Encoding($false)))
Write-Host "vcpkg baseline -> $vcpkgHead"

# ---- 2. bump CommonLib in the overlay port (same edits as ci/build-oif.ps1) --
$port = 'cmake\ports\commonlibsse-ng\portfile.cmake'
$sha512 = Get-GitHubArchiveSha512 -Repo 'alandtse/CommonLibVR' -Sha $CommonLibRef
Edit-File $port 'REF e617713b2ae8a927bf925d1ad138cc48ab72e414' "REF $CommonLibRef"
Edit-File $port 'SHA512 997238522e1433dd81c73b4762ae8ec0e919ac65604570924ae5a76ba7d4428dfe8d961cf1fdc693744dba6be66bf45ba16d6a603ca9bb8234f08aedf2df146d' "SHA512 $sha512"
Edit-File $port "vcpkg_configure_cmake(`n    SOURCE_PATH `"`${SOURCE_PATH}`"`n    PREFER_NINJA`n    OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on`n)" "vcpkg_cmake_configure(`n    SOURCE_PATH `"`${SOURCE_PATH}`"`n    OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on`n)"
Edit-File $port 'vcpkg_install_cmake()' 'vcpkg_cmake_install()'
$minhookSha512 = Get-GitHubArchiveSha512 -Repo 'TsudaKageyu/minhook' -Sha 'v1.3.4'
Edit-File $port "file(COPY `${OPENVR_FILES} DESTINATION `"`${SOURCE_PATH}/extern/openvr`")" "file(COPY `${OPENVR_FILES} DESTINATION `"`${SOURCE_PATH}/extern/openvr`")`n`nvcpkg_from_github(`n    OUT_SOURCE_PATH MINHOOK_SOURCE_PATH`n    REPO TsudaKageyu/minhook`n    REF v1.3.4`n    SHA512 $minhookSha512`n    HEAD_REF master`n)"
Edit-File $port 'OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on' "OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on `"-DFETCHCONTENT_SOURCE_DIR_HDE64=`${MINHOOK_SOURCE_PATH}`""
Edit-File $port 'INSTALL "${SOURCE_PATH}/LICENSE"' 'INSTALL "${SOURCE_PATH}/COPYING.txt"'
Get-Content $port
$portJson = 'cmake\ports\commonlibsse-ng\vcpkg.json'
Edit-File $portJson '"version-semver": "3.7.0"' '"version-semver": "8.0.1"'
Edit-File $portJson "`"dependencies`": [`n    {`n      `"name`": `"vcpkg-cmake-config`",`n      `"host`": true`n    }," "`"dependencies`": [`n    {`n      `"name`": `"vcpkg-cmake`",`n      `"host`": true`n    },`n    {`n      `"name`": `"vcpkg-cmake-config`",`n      `"host`": true`n    },"
Edit-File $portJson "`"default-features`": [`n    `"tests`"`n  ]" "`"default-features`": []"

# The plugin source is already NG 8.x. Confirm the two calls, and refuse the 3.7 spellings.
$main = [IO.File]::ReadAllText((Join-Path $src 'src\main.cpp'))
if ($main -notmatch 'UsesAddressLibrary\(\)') { throw 'src/main.cpp is missing UsesAddressLibrary() (no-arg, NG 8.x)' }
if ($main -match 'UsesAddressLibrary\(true\)') { throw 'src/main.cpp still uses the NG 3.7 UsesAddressLibrary(true) spelling' }
if ($main -notmatch 'UsesNoStructs\(\)') { throw 'src/main.cpp is missing UsesNoStructs() (NG 8.x)' }
if ($main -match 'HasNoStructUse') { throw 'src/main.cpp still uses the NG 3.7 HasNoStructUse spelling' }
$patches = @(
    'No source patch to src/main.cpp or src/PCH.h. Both already target CommonLibSSE-NG 8.x: UsesAddressLibrary() takes no argument, UsesNoStructs() is the renamed no-struct flag, and PCH.h defines NOMINMAX before the Windows headers.',
    'CMakeLists.txt links CommonLibSSE::CommonLibSSE directly. add_commonlibsse_plugin() would emit a second SKSEPlugin_Version and fail the link.',
    "cmake/ports/commonlibsse-ng: REF/SHA512 bumped to alandtse/CommonLibVR $CommonLibRef, vcpkg_cmake_* helpers, MinHook fetched for SKSE_SUPPORT_PATCH_SAFETY, copyright file is COPYING.txt, port version 8.0.1, tests feature off.",
    "vcpkg.json builtin-baseline set to microsoft/vcpkg $vcpkgHead."
)

# ---- 3. configure + build (preset "AE") --------------------------------------
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
$triplets = New-ReleaseOnlyTriplet -Name 'x64-windows-static-md'
try {
    Invoke-Checked cmake --preset AE "-DVCPKG_OVERLAY_TRIPLETS=$triplets"
} catch {
    Show-VcpkgFailureLogs -Port 'commonlibsse-ng'
    throw
}
try {
    Invoke-Checked cmake --build --preset AE
} catch {
    Show-VcpkgFailureLogs -Port 'commonlibsse-ng'
    throw
}
$dll = Get-ChildItem -Recurse -Filter AutoPlantEverything.dll build | Select-Object -First 1
if (-not $dll) { throw 'AutoPlantEverything.dll not produced' }
Write-Host "built $($dll.FullName) ($($dll.Length) bytes)"
if ($dll.Length -lt 50000) {
    throw "AutoPlantEverything.dll is only $($dll.Length) bytes - that is not a real build"
}

# ---- 4. version block: address library + no structs, x64, real exports -------
$verify = @'
import sys, struct, pefile
p = sys.argv[1]
data = open(p, "rb").read()
pe = pefile.PE(data=data)
assert pe.FILE_HEADER.Machine == 0x8664, "not x64"
exports = {e.name.decode(): e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
assert "SKSEPlugin_Load" in exports, exports
assert "SKSEPlugin_Version" in exports, exports
off = pe.get_offset_from_rva(exports["SKSEPlugin_Version"])
blk = data[off:off + 0x350]
data_version, plugin_version = struct.unpack_from("<II", blk, 0)
name = blk[8:264].split(b"\0", 1)[0].decode()
author = blk[264:520].split(b"\0", 1)[0].decode()
indep_ex, indep = struct.unpack_from("<II", blk, 0x304)
print(f"name={name!r} author={author!r} pluginVersion={plugin_version:#x} versionIndependenceEx={indep_ex:#x} versionIndependence={indep:#x} dataVersion={data_version}")
assert data_version == 1, data_version
assert name == "AutoPlantEverything", name
assert author == "Jo Hoeflich", author
assert plugin_version == 0x00020000, hex(plugin_version)
# kVersionIndependentEx_AddressLibraryV5 (1<<1) | kVersionIndependentEx_NoStructUse (1<<0)
assert indep_ex == 0x3, hex(indep_ex)
# kVersionIndependent_AddressLibraryPostAE
assert indep == 0x1, hex(indep)
print("version block OK")
'@
$verifyPath = Join-Path $src 'verify_version_block.py'
$verify | Out-File -Encoding ascii $verifyPath
Invoke-Checked python '-m' pip install --quiet pefile
Invoke-Checked python $verifyPath $dll.FullName

# ---- 5. player zip: DLL + INI only. No FOMOD, no ESL, no ESP. ----------------
$ini = Join-Path $src 'package\SKSE\Plugins\AutoPlantEverything.ini'
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Force $dll.FullName (Join-Path $OutDir 'AutoPlantEverything.dll')
Copy-Item -Force $ini (Join-Path $OutDir 'AutoPlantEverything.ini')
Copy-Item -Force (Join-Path $src 'LICENSE') (Join-Path $OutDir 'LICENSE')
$pdb = $dll.FullName -replace '\.dll$', '.pdb'
if (Test-Path $pdb) { Copy-Item -Force $pdb (Join-Path $OutDir 'AutoPlantEverything.pdb') }

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zipPath = Join-Path $OutDir 'AutoPlantEverything-0.2.0.zip'
if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
$zip = [System.IO.Compression.ZipFile]::Open($zipPath, 'Create')
try {
    foreach ($pair in @(
            @{ Dest = 'SKSE/Plugins/AutoPlantEverything.dll'; Src = $dll.FullName },
            @{ Dest = 'SKSE/Plugins/AutoPlantEverything.ini'; Src = $ini }
        )) {
        $entry = $zip.CreateEntry($pair.Dest, [System.IO.Compression.CompressionLevel]::Optimal)
        $inStream = [System.IO.File]::OpenRead($pair.Src)
        try {
            $outStream = $entry.Open()
            try { $inStream.CopyTo($outStream) } finally { $outStream.Dispose() }
        } finally { $inStream.Dispose() }
    }
} finally { $zip.Dispose() }

$packed = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
try {
    $names = @($packed.Entries | ForEach-Object { $_.FullName.Replace('\', '/') })
    Write-Host ("zip entries: " + ($names -join ', '))
    $expected = @('SKSE/Plugins/AutoPlantEverything.dll', 'SKSE/Plugins/AutoPlantEverything.ini')
    if (($names | Sort-Object) -join '|' -ne (($expected | Sort-Object) -join '|')) {
        throw "player zip must contain only the DLL and the INI, found: $($names -join ', ')"
    }
    foreach ($n in $names) {
        if ($n -match 'ModuleConfig|\.esl$|\.esp$|\.esm$') { throw "player zip contains a forbidden entry: $n" }
    }
    $dllEntry = $packed.Entries | Where-Object { $_.FullName.Replace('\','/') -eq 'SKSE/Plugins/AutoPlantEverything.dll' }
    if ($dllEntry.Length -lt 50000) { throw "zipped DLL is only $($dllEntry.Length) bytes" }
} finally { $packed.Dispose() }
Write-Host "wrote $zipPath ($((Get-Item $zipPath).Length) bytes)"

New-SourceZip -SrcDir $src -ZipPath (Join-Path $OutDir 'AutoPlantEverything-0.2.0-1.7.104-src.zip') -Readme @"
Auto Plant Everything 0.2.0 as built for Skyrim AE 1.7.104 - corresponding source.
Plugin source: this repository, sources/autoplanteverything (Jo Hoeflich).
CommonLib is fetched by vcpkg from the overlay port in cmake/ports/commonlibsse-ng:
  https://github.com/alandtse/CommonLibVR commit $CommonLibRef (branch ng, CommonLibSSE-NG 8.0.1, GPL-3.0-or-later with Modding Exception)
Build: cmake --preset AE ; cmake --build --preset AE   (VS 2022, vcpkg $vcpkgHead, triplet x64-windows-static-md)
"@
Pop-Location

Write-BuildNotes -OutDir $OutDir -ModName 'Auto Plant Everything' -Dll 'AutoPlantEverything.dll' -Facts ([ordered]@{
    'Source'            = 'sources/autoplanteverything in skyrim-community-rebuild-factory (original plugin, version 0.2.0)'
    'Author'            = 'Jo Hoeflich'
    'Plugin licence'    = 'MIT for the plugin source (LICENSE alongside). The DLL is GPL-3.0-or-later because it links CommonLibSSE-NG.'
    'CommonLib'         = "alandtse/CommonLibVR branch ng, commit ``$CommonLibRef`` (CommonLibSSE-NG 8.0.1) via cmake/ports/commonlibsse-ng (REF/SHA512 bumped)"
    'vcpkg'             = "microsoft/vcpkg ``$vcpkgHead``, triplet x64-windows-static-md"
    'Configure'         = '``cmake --preset AE`` (Visual Studio 17 2022, x64, /MD)'
    'Build'             = '``cmake --build --preset AE`` (Release)'
    'Player zip'        = '``AutoPlantEverything-0.2.0.zip`` — SKSE/Plugins/AutoPlantEverything.dll + AutoPlantEverything.ini only'
}) -Patches $patches -Caveats @(
    'Not smoke-tested in Skyrim. A DLL that links and exports the address-library version block can still fail to load or mis-plant. Jo has to SKSE-boot 1.7.104 and check Documents\My Games\Skyrim Special Edition\SKSE\AutoPlantEverything.log before this replaces anything.',
    'Do not remove or replace Grokbot-AllPlantablePlants.esp. That plugin is the live planter fix (185 pairs). This DLL is an additional SKSE plugin, not a substitute, until it has been tested in-game.',
    'tools/generate_esl.py refuses empty and shrinking FormList overrides. CI checks that refusal. Nothing in this build writes an ESL or ESP.'
)
Get-ChildItem $OutDir | Format-Table Name, Length
# robocopy (inside New-SourceZip) exits 1 when it copied files; the runner would otherwise report that as the step result
$global:LASTEXITCODE = 0
