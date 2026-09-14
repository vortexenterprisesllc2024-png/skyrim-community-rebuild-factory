# Description Framework 2.1.2 (Nightfallstorm, GPL-3.0) rebuilt for Skyrim AE 1.7.104 / Address Library format 5.
# Recipe: scripts/description-framework-rebuild.md. CommonLib is the git submodule extern/CommonLibSSE-NG,
# bumped from 738afc4 (NG 3.6.0, no SSEv5) to an SSEv5-capable alandtse/CommonLibVR ng commit.
param(
    [string]$CommonLibRef = 'd13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd',
    [string]$UpstreamTag = '2.1.2',
    [string]$VcpkgRef = 'master',
    [string]$OutDir = "$env:GITHUB_WORKSPACE\releases\1.7.104\description-framework"
)
. "$PSScriptRoot\common.ps1"

$src = "$env:RUNNER_TEMP\src\DescriptionFramework"
$vcpkgHead = Initialize-Vcpkg -Ref $VcpkgRef

# ---- 1. clone + pin ----------------------------------------------------------
if (-not (Test-Path "$src\.git")) {
    Invoke-Checked git clone https://github.com/Nightfallstorm/DescriptionFramework.git $src
}
Push-Location $src
Invoke-Checked git fetch --tags --quiet
Invoke-Checked git checkout --quiet $UpstreamTag
$upstreamSha = (git rev-parse HEAD).Trim()
Write-Host "DescriptionFramework $UpstreamTag = $upstreamSha"

# ---- 2. bump the CommonLib submodule -----------------------------------------
$clib = Get-CommonLib -Ref $CommonLibRef -Dest "$src\extern\CommonLibSSE-NG"

# ---- 3. vcpkg manifest: 2023 baseline + fmt 8.0.1 override cannot feed NG 8.x (needs fmt>=12.1, spdlog>=1.16)
$manifest = Get-Content vcpkg.json -Raw | ConvertFrom-Json
$manifest.'builtin-baseline' = $vcpkgHead
$manifest.PSObject.Properties.Remove('overrides')
$manifest | ConvertTo-Json -Depth 8 | Out-File -Encoding utf8 vcpkg.json
Get-Content vcpkg.json

# ---- 4. source patches --------------------------------------------------------
$patches = @()
Edit-File 'src\main.cpp' 'SKSE::Init(a_skse);' 'SKSE::Init(a_skse, { .log = false });'
$patches += 'src/main.cpp: SKSE::Init(a_skse) in CommonLibSSE-NG 8.x installs its own spdlog logger by default, which would replace the logger InitializeLog() already set up (and drop the bDebug level); { .log = false } keeps the original DescriptionFramework.log behaviour.'
Edit-File 'src\main.cpp' 'v.UsesAddressLibrary(true);' 'v.UsesAddressLibrary();'
Edit-File 'src\main.cpp' 'v.UsesNoStructs(true);' 'v.UsesNoStructs();'
$patches += 'src/main.cpp: PluginVersionData::UsesAddressLibrary() / UsesNoStructs() take no argument in NG 8.x (same flags set).'
Edit-File 'src\PCH.h' '#include "SKSE/SKSE.h"' "#include `"SKSE/SKSE.h`"`n#ifndef SKSEAPI`n#`tdefine SKSEAPI __cdecl`n#endif"
$patches += 'src/PCH.h: NG 8.x no longer defines the SKSEAPI calling-convention macro; defined as __cdecl (a no-op on x64) so the SKSEPlugin_Query signature compiles unchanged.'
Edit-File 'src\PCH.h' '#define WIN32_LEAN_AND_MEAN' "#define NOMINMAX`n#define WIN32_LEAN_AND_MEAN"
$patches += 'src/PCH.h: NOMINMAX before the Windows headers - with NG 8.x the min/max macros leak into SimpleIni.h (std::numeric_limits<>::max()) and break the build.'
$patches += "vcpkg.json: builtin-baseline moved from 417119555f155f6044dec7a379cd25466e339873 (2023) to $vcpkgHead and the fmt 8.0.1 override removed, because NG 8.0.1 requires fmt >= 12.1.0 / spdlog >= 1.16.0. No dependency added or removed."
$patches += "extern/CommonLibSSE-NG: submodule 738afc457fdeba050ce328f15225aa3cde674187 (NG 3.6.0, formats 1/2 only) -> $($clib.Sha) (NG $($clib.Version), Format::SSEv5)."
Invoke-Checked git --no-pager diff --stat

# ---- 5. configure + build (upstream preset "descriptionframework") -------------
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
$triplets = New-ReleaseOnlyTriplet -Name 'x64-windows-static'
try {
    Invoke-Checked cmake --preset descriptionframework -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON "-DVCPKG_OVERLAY_TRIPLETS=$triplets"
} catch {
    Show-VcpkgFailureLogs
    throw
}
Invoke-Checked cmake --build build --config Release
$dll = Get-ChildItem -Recurse -Filter DescriptionFramework.dll build | Select-Object -First 1
if (-not $dll) { throw 'DescriptionFramework.dll not produced' }
Write-Host "built $($dll.FullName) ($($dll.Length) bytes)"

# ---- 6. package (+ GPL corresponding source) ----------------------------------
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Force $dll.FullName $OutDir
Copy-Item -Force LICENSE $OutDir
if (Test-Path ($dll.FullName -replace '\.dll$', '.pdb')) { Copy-Item -Force ($dll.FullName -replace '\.dll$', '.pdb') $OutDir }
New-SourceZip -SrcDir $src -ZipPath (Join-Path $OutDir "DescriptionFramework-$UpstreamTag-1.7.104-rebuild-src.zip") -Readme @"
Description Framework $UpstreamTag (commit $upstreamSha) as rebuilt for Skyrim AE 1.7.104 - corresponding source (GPL-3.0).
Upstream: https://github.com/Nightfallstorm/DescriptionFramework (GPL-3.0-or-later)
This tree = upstream tag $UpstreamTag + the patches listed in BUILD-NOTES.md, with extern/CommonLibSSE-NG checked out at
  https://github.com/alandtse/CommonLibVR commit $($clib.Sha) (branch ng, CommonLibSSE-NG $($clib.Version), GPL-3.0-or-later with Modding Exception) - unmodified,
  whose own extern/openvr submodule (https://github.com/ValveSoftware/openvr, unmodified) is left out of the zip for size.
Build: cmake --preset descriptionframework -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON ; cmake --build build --config Release
       (VS 2022, vcpkg $vcpkgHead, triplet x64-windows-static)
"@
Pop-Location

Write-BuildNotes -OutDir $OutDir -ModName 'Description Framework' -Dll 'DescriptionFramework.dll' -Facts ([ordered]@{
    'Upstream'          = "https://github.com/Nightfallstorm/DescriptionFramework tag ``$UpstreamTag`` (commit ``$upstreamSha``)"
    'Upstream licence'  = 'GPL-3.0-or-later (LICENSE alongside; corresponding source in the *-rebuild-src.zip)'
    'Nexus'             = 'https://www.nexusmods.com/skyrimspecialedition/mods/105799'
    'CommonLib'         = "alandtse/CommonLibVR branch ng, commit ``$($clib.Sha)`` (CommonLibSSE-NG $($clib.Version)) as submodule extern/CommonLibSSE-NG"
    'vcpkg'             = "microsoft/vcpkg ``$vcpkgHead``, triplet x64-windows-static"
    'Configure'         = '``cmake --preset descriptionframework -DBUILD_TESTS=OFF -DSKSE_SUPPORT_XBYAK=ON`` (Visual Studio 17 2022, v143, /MT)'
    'Build'             = '``cmake --build build --config Release``'
}) -Patches $patches -Caveats @(
    'The six ItemCard hooks are call-site hooks at fixed offsets inside functions resolved by Address Library ID (e.g. RELOCATION_ID(50674, 51569) + 0x7A). Those offsets date from 1.6.x. If the game boots clean but item cards show no description text, or crash on opening a menu, that is hook drift on 1.7.104 - not a format-5 problem.',
    'Upstream logs a critical warning if po3_Tweaks is not loaded; unchanged here.',
    'Replace only DescriptionFramework.dll; keep the existing *_DESC.ini description packs.',
    'Prefer an official Nightfallstorm 1.7.104 build if one appears; then this rebuild is superseded.'
)
Get-ChildItem $OutDir | Format-Table Name, Length
