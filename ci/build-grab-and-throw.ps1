# Grab And Throw v2.1.2 (powerof3, MIT) rebuilt for Skyrim AE 1.7.104 / Address Library format 5.
# Recipe: scripts/grab-and-throw-rebuild.md. Upstream links powerof3/CommonLibSSE (submodule, Jan 2026,
# no format 5). We leave that submodule uninitialised and point $env:CommonLibSSEPath at an SSEv5-capable
# alandtse/CommonLibVR ng checkout, which upstream CMakeLists.txt already supports as its fallback.
param(
    [string]$CommonLibRef = 'd13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd',
    [string]$UpstreamTag = 'v2.1.2',
    [string]$VcpkgRef = 'master',
    [string]$OutDir = "$env:GITHUB_WORKSPACE\releases\1.7.104\grab-and-throw"
)
. "$PSScriptRoot\common.ps1"

$srcRoot = "$env:RUNNER_TEMP\src"
$src = "$srcRoot\GrabAndThrow"
$clibDir = "$srcRoot\CommonLibVR"
$vcpkgHead = Initialize-Vcpkg -Ref $VcpkgRef

# ---- 1. clone + pin (no submodules on purpose) --------------------------------
if (-not (Test-Path "$src\.git")) {
    Invoke-Checked git clone https://github.com/powerof3/GrabAndThrow.git $src
}
Push-Location $src
Invoke-Checked git fetch --tags --quiet
Invoke-Checked git checkout --quiet $UpstreamTag
$upstreamSha = (git rev-parse HEAD).Trim()
Write-Host "GrabAndThrow $UpstreamTag = $upstreamSha"

# ---- 2. CommonLib: alandtse ng via CommonLibSSEPath ---------------------------
$clib = Get-CommonLib -Ref $CommonLibRef -Dest $clibDir
$env:CommonLibSSEPath = $clibDir

# ---- 3. vcpkg manifest: NG needs directxtk + rapidcsv, and a 2026 baseline ------
$manifest = Get-Content vcpkg.json -Raw | ConvertFrom-Json
$manifest.'builtin-baseline' = $vcpkgHead
$deps = [System.Collections.ArrayList]@($manifest.dependencies)
foreach ($d in 'directxtk', 'rapidcsv') { if ($deps -notcontains $d) { [void]$deps.Add($d) } }
$manifest.dependencies = $deps.ToArray()
$manifest | ConvertTo-Json -Depth 8 | Out-File -Encoding utf8 vcpkg.json
Get-Content vcpkg.json

# ---- 4. source patches (powerof3 CommonLibSSE -> CommonLibSSE-NG) --------------
$patches = @()
Edit-File 'src\Hooks.cpp' 'RE::PlayerCharacter::GetSingleton()->grabType' 'RE::PlayerCharacter::GetSingleton()->GetPlayerRuntimeData().grabType'
Edit-File 'src\Hooks.cpp' 'player->grabType' 'player->GetPlayerRuntimeData().grabType'
$patches += 'src/Hooks.cpp: PlayerCharacter::grabType is a direct member in powerof3/CommonLibSSE but lives in the versioned runtime-data block in CommonLibSSE-NG; read it through GetPlayerRuntimeData() (two call sites, same field, same GrabbingType compare).'
Edit-File 'src\GrabThrowHandler.cpp' 'RE::PlayerCharacter::GetSingleton()->currentProcess' 'RE::PlayerCharacter::GetSingleton()->GetActorRuntimeData().currentProcess'
Edit-File 'src\GrabThrowHandler.cpp' 'player->currentProcess->SetActorsDetectionEvent(' 'player->GetActorRuntimeData().currentProcess->SetActorsDetectionEvent('
Edit-File 'src\GrabThrowHandler.cpp' 'a_player->grabSpring' 'a_player->GetPlayerRuntimeData().grabSpring'
$patches += 'src/GrabThrowHandler.cpp: Actor::currentProcess and PlayerCharacter::grabSpring likewise live in NG runtime-data blocks; read through GetActorRuntimeData() / GetPlayerRuntimeData() (three call sites, same fields).'
Edit-File 'src\PCH.h' '#include "SKSE/SKSE.h"' "#include `"SKSE/SKSE.h`"`n#ifndef SKSEAPI`n#`tdefine SKSEAPI __cdecl`n#endif"
$patches += 'src/PCH.h: NG 8.x no longer defines the SKSEAPI calling-convention macro; defined as __cdecl (a no-op on x64) so the SKSEPlugin_Load/Query signatures compile unchanged.'
$patches += "vcpkg.json: builtin-baseline 14bb451131ccf6be50a63a8d9dfe7980e46b5958 -> $vcpkgHead; added directxtk and rapidcsv, which CommonLibSSE-NG's CMakeLists requires (powerof3's fork did not). Existing deps (clib-util, rsm-binary-io, spdlog, xbyak) untouched."
$patches += "CommonLib: extern/CommonLibSSE submodule (powerof3/CommonLibSSE 7312db81, no format 5) left uninitialised; CommonLibSSEPath=$clibDir -> alandtse/CommonLibVR $($clib.Sha) (NG $($clib.Version), Format::SSEv5). Upstream CMakeLists already resolves CommonLib from that env var when extern/ is empty."
Invoke-Checked git --no-pager diff --stat

# ---- 5. configure + build (upstream preset "vs2022-windows-vcpkg-ae") -----------
Remove-Item -Recurse -Force buildae -ErrorAction SilentlyContinue
$triplets = New-ReleaseOnlyTriplet -Name 'x64-windows-static'
try {
    Invoke-Checked cmake --preset vs2022-windows-vcpkg-ae -DCOPY_BUILD=OFF -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON "-DVCPKG_OVERLAY_TRIPLETS=$triplets"
} catch {
    Show-VcpkgFailureLogs
    throw
}
Invoke-Checked cmake --build buildae --config Release
$dll = Get-ChildItem -Recurse -Filter po3_GrabAndThrow.dll buildae | Select-Object -First 1
if (-not $dll) { throw 'po3_GrabAndThrow.dll not produced' }
Write-Host "built $($dll.FullName) ($($dll.Length) bytes)"

# ---- 6. package ---------------------------------------------------------------
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Force $dll.FullName $OutDir
Copy-Item -Force LICENSE $OutDir
if (Test-Path ($dll.FullName -replace '\.dll$', '.pdb')) { Copy-Item -Force ($dll.FullName -replace '\.dll$', '.pdb') $OutDir }
Pop-Location
# corresponding source = GrabAndThrow (patched) + the CommonLibVR checkout it was linked against, side by side
New-SourceZip -SrcDir $srcRoot -ZipPath (Join-Path $OutDir "GrabAndThrow-$UpstreamTag-1.7.104-rebuild-src.zip") -Readme @"
Grab And Throw $UpstreamTag (commit $upstreamSha) as rebuilt for Skyrim AE 1.7.104 - corresponding source.
Upstream: https://github.com/powerof3/GrabAndThrow (MIT)
GrabAndThrow/ = upstream tag $UpstreamTag + the patches listed in BUILD-NOTES.md (extern/CommonLibSSE submodule intentionally empty).
CommonLibVR/  = https://github.com/alandtse/CommonLibVR commit $($clib.Sha) (branch ng, CommonLibSSE-NG $($clib.Version), GPL-3.0-or-later with Modding Exception) - unmodified;
                its extern/openvr submodule (https://github.com/ValveSoftware/openvr, unmodified) is left out of the zip for size.
Build: set CommonLibSSEPath=<path to CommonLibVR> ; cmake --preset vs2022-windows-vcpkg-ae -DCOPY_BUILD=OFF -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON ; cmake --build buildae --config Release
       (VS 2022, vcpkg $vcpkgHead, triplet x64-windows-static)
"@

Write-BuildNotes -OutDir $OutDir -ModName 'Grab And Throw' -Dll 'po3_GrabAndThrow.dll' -Facts ([ordered]@{
    'Upstream'          = "https://github.com/powerof3/GrabAndThrow tag ``$UpstreamTag`` (commit ``$upstreamSha``)"
    'Upstream licence'  = 'MIT (LICENSE alongside)'
    'Nexus'             = 'https://www.nexusmods.com/skyrimspecialedition/mods/120460'
    'CommonLib'         = "alandtse/CommonLibVR branch ng, commit ``$($clib.Sha)`` (CommonLibSSE-NG $($clib.Version)) via CommonLibSSEPath (upstream submodule powerof3/CommonLibSSE not used)"
    'vcpkg'             = "microsoft/vcpkg ``$vcpkgHead``, triplet x64-windows-static"
    'Configure'         = '``cmake --preset vs2022-windows-vcpkg-ae -DCOPY_BUILD=OFF -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON`` (Visual Studio 17 2022, v143, /MT, SKYRIM_AE)'
    'Build'             = '``cmake --build buildae --config Release``'
}) -Patches $patches -Caveats @(
    'Three call-site hooks use fixed offsets inside Address-Library-resolved functions (44005+0x50, 38185+0x200, 25850+0xF4) from the 1.6.x era. If boot is clean but grabbing/throwing crashes, that is hook drift on 1.7.104 - report the faulting address.',
    'Built against CommonLibSSE-NG instead of powerof3 own CommonLibSSE fork: struct layouts for AE 1.7.x come from NG 8.0.1 (which has explicit 1.7.99+ layout work). Same engine functions, different RE library - test hold-E grab, R throw, and an OIF "Throw" rule.',
    'Replace only po3_GrabAndThrow.dll; keep the existing po3_GrabAndThrow.ini.',
    'Prefer an official powerof3 1.7.104 build if one appears; then this rebuild is superseded.'
)
Get-ChildItem $OutDir | Format-Table Name, Length
