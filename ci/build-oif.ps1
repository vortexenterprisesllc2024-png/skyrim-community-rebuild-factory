# Object Impact Framework 1.6.8 (arachnocid, Apache-2.0) rebuilt for Skyrim AE 1.7.104 / Address Library format 5.
# Recipe: scripts/oif-rebuild.md. CommonLib comes in through the upstream vcpkg overlay port
# cmake/ports/commonlibsse-ng/portfile.cmake; we only bump REF/SHA512 to an SSEv5-capable ng commit.
param(
    [string]$CommonLibRef = 'd13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd',
    [string]$UpstreamTag = '1.6.8',
    [string]$VcpkgRef = 'master',
    [string]$OutDir = "$env:GITHUB_WORKSPACE\releases\1.7.104\oif"
)
. "$PSScriptRoot\common.ps1"

$src = "$env:RUNNER_TEMP\src\ObjectImpactFramework"
$vcpkgHead = Initialize-Vcpkg -Ref $VcpkgRef

# ---- 1. clone + pin ----------------------------------------------------------
if (-not (Test-Path "$src\.git")) {
    Invoke-Checked git clone https://github.com/arachnocid/ObjectImpactFramework.git $src
}
Push-Location $src
Invoke-Checked git fetch --tags --quiet
Invoke-Checked git checkout --quiet $UpstreamTag
$upstreamSha = (git rev-parse HEAD).Trim()
Write-Host "ObjectImpactFramework $UpstreamTag = $upstreamSha"

# ---- 2. bump CommonLib in the overlay port -----------------------------------
$port = 'cmake\ports\commonlibsse-ng\portfile.cmake'
$sha512 = Get-GitHubArchiveSha512 -Repo 'alandtse/CommonLibVR' -Sha $CommonLibRef
Edit-File $port 'REF e617713b2ae8a927bf925d1ad138cc48ab72e414' "REF $CommonLibRef"
Edit-File $port 'SHA512 997238522e1433dd81c73b4762ae8ec0e919ac65604570924ae5a76ba7d4428dfe8d961cf1fdc693744dba6be66bf45ba16d6a603ca9bb8234f08aedf2df146d' "SHA512 $sha512"
# old vcpkg_configure_cmake / vcpkg_install_cmake are the deprecated names; use the current ones.
Edit-File $port "vcpkg_configure_cmake(`n    SOURCE_PATH `"`${SOURCE_PATH}`"`n    PREFER_NINJA`n    OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on`n)" "vcpkg_cmake_configure(`n    SOURCE_PATH `"`${SOURCE_PATH}`"`n    OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on`n)"
Edit-File $port 'vcpkg_install_cmake()' 'vcpkg_cmake_install()'
# NG 8.x vendors MinHook's hde64 through FetchContent (SKSE_SUPPORT_PATCH_SAFETY). vcpkg configures ports with
# FETCHCONTENT_FULLY_DISCONNECTED=ON, so fetch MinHook the vcpkg way and hand it to FetchContent by path.
$minhookSha512 = Get-GitHubArchiveSha512 -Repo 'TsudaKageyu/minhook' -Sha 'v1.3.4'
Edit-File $port "file(COPY `${OPENVR_FILES} DESTINATION `"`${SOURCE_PATH}/extern/openvr`")" "file(COPY `${OPENVR_FILES} DESTINATION `"`${SOURCE_PATH}/extern/openvr`")`n`nvcpkg_from_github(`n    OUT_SOURCE_PATH MINHOOK_SOURCE_PATH`n    REPO TsudaKageyu/minhook`n    REF v1.3.4`n    SHA512 $minhookSha512`n    HEAD_REF master`n)"
Edit-File $port 'OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on' "OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on `"-DFETCHCONTENT_SOURCE_DIR_HDE64=`${MINHOOK_SOURCE_PATH}`""
# NG 8.x ships COPYING.txt (GPL-3.0-or-later + exceptions), not LICENSE
Edit-File $port 'INSTALL "${SOURCE_PATH}/LICENSE"' 'INSTALL "${SOURCE_PATH}/COPYING.txt"'
Get-Content $port
$portJson = 'cmake\ports\commonlibsse-ng\vcpkg.json'
Edit-File $portJson '"version-semver": "3.7.0"' '"version-semver": "8.0.1"'
Edit-File $portJson "`"dependencies`": [`n    {`n      `"name`": `"vcpkg-cmake-config`",`n      `"host`": true`n    }," "`"dependencies`": [`n    {`n      `"name`": `"vcpkg-cmake`",`n      `"host`": true`n    },`n    {`n      `"name`": `"vcpkg-cmake-config`",`n      `"host`": true`n    },"
# don't drag catch2 in for a library we never test here
Edit-File $portJson "`"default-features`": [`n    `"tests`"`n  ]" "`"default-features`": []"

# ---- 3. source patches for CommonLibSSE-NG 3.7 -> 8.0 API -------------------
$patches = @()
Edit-File 'include\PCH.h' 'v.HasNoStructUse();' 'v.UsesNoStructs();'
$patches += 'include/PCH.h: SKSE::PluginVersionData::HasNoStructUse() was renamed UsesNoStructs() in CommonLibSSE-NG (same flag, kVersionIndependentEx_NoStructUse).'
Edit-File 'include\PCH.h' 'SKSE::Init(a_skse);' 'SKSE::Init(a_skse, { .log = false });'
$patches += 'include/PCH.h: SKSE::Init(a_skse) now installs its own spdlog logger by default; pass { .log = false } so the plugin keeps its own ObjectImpactFramework.log setup (same file name, original pattern).'
Edit-File 'include\PCH.h' 'v.UsesAddressLibrary(true);' 'v.UsesAddressLibrary();'
$patches += 'include/PCH.h: PluginVersionData::UsesAddressLibrary() takes no argument in NG 8.x (same flag set).'
Edit-File 'include\PCH.h' "#`tinclude `"SKSE/SKSE.h`"" "#`tinclude `"SKSE/SKSE.h`"`n#ifndef SKSEAPI`n#`tdefine SKSEAPI __cdecl`n#endif"
$patches += 'include/PCH.h: NG 8.x no longer defines the SKSEAPI calling-convention macro; defined as __cdecl (a no-op on x64) so the SKSEPlugin_Load/Query signatures compile unchanged.'
Edit-File 'include\PCH.h' "#`tdefine SKSE_SUPPORT_XBYAK" "#define NOMINMAX`n#`tdefine SKSE_SUPPORT_XBYAK"
$patches += 'include/PCH.h: NOMINMAX before the Windows headers so std::numeric_limits<>::max() in NG 8.x / dependency headers is not eaten by the min/max macros (OIF code already parenthesises its own uses).'
Edit-File 'src\Effects.cpp' 'RE::DebugNotification(' 'RE::SendHUDMessage::ShowHUDMessage('
$patches += 'src/Effects.cpp: RE::DebugNotification() no longer exists in NG 8.x; RE::SendHUDMessage::ShowHUDMessage() is the same engine call (same three arguments).'
Edit-File 'src\EventSinks.cpp' 'RE::MagicSystem::Delivery::kTotal' 'RE::MagicSystem::Delivery::kNone' -Expect 3
$patches += 'src/EventSinks.cpp: MagicSystem::Delivery::kTotal (value 5) is spelled kNone (value 5) in NG 8.x - three switch cases, same value.'
Edit-File 'src\Effects.cpp' 'RE::COL_LAYER::kClutterLarge,' 'static_cast<RE::COL_LAYER>(29), // NG 3.7 named layer 29 kClutterLarge; NG 8.x names it kWard - value kept'
Edit-File 'src\Effects.cpp' 'RE::COL_LAYER::kItemPicker,' 'RE::COL_LAYER::kItemPick, // NG 8.x spelling of kItemPicker (40)'
$patches += 'src/Effects.cpp: two COL_LAYER enumerators renamed in NG 8.x (kClutterLarge=29 -> written as the literal 29; kItemPicker=40 -> kItemPick); the raycast layer list keeps the same numeric layers.'
Edit-File 'src\Effects.cpp' "uint32_t filterInfo = 0;`n                        sourceActor->GetCollisionFilterInfo(filterInfo);" "RE::CFilter collisionFilter{};`n                        sourceActor->GetCollisionFilterInfo(collisionFilter);`n                        uint32_t filterInfo = collisionFilter.filter;"
Edit-File 'src\Effects.cpp' 'pick.rayInput.filterInfo = (filterInfo & 0xFFFF0000) | static_cast<uint32_t>(layer);' 'pick.rayInput.filterInfo.filter = (filterInfo & 0xFFFF0000) | static_cast<uint32_t>(layer);'
$patches += 'src/Effects.cpp: Actor::GetCollisionFilterInfo() and hkpWorldRayCastInput::filterInfo now use the RE::CFilter wrapper (a struct around the same uint32) instead of a raw uint32_t; same engine call (ID 36559/37560), same bits.'
Edit-File 'src\Effects.cpp' 'audioManager->BuildSoundDataFromDescriptor(' 'audioManager->GetSoundHandle('
$patches += 'src/Effects.cpp: BSAudioManager::BuildSoundDataFromDescriptor() was renamed GetSoundHandle() in NG 8.x - same engine function (ID 66404/67666), same arguments.'
Invoke-Checked git --no-pager diff --stat

# ---- 4. configure + build (upstream preset "AE") ------------------------------
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
$triplets = New-ReleaseOnlyTriplet -Name 'x64-windows-static-md'
try {
    Invoke-Checked cmake --preset AE "-DVCPKG_OVERLAY_TRIPLETS=$triplets"
} catch {
    Show-VcpkgFailureLogs -Port 'commonlibsse-ng'
    throw
}
Invoke-Checked cmake --build --preset AE
$dll = Get-ChildItem -Recurse -Filter ObjectImpactFramework.dll build | Select-Object -First 1
if (-not $dll) { throw 'ObjectImpactFramework.dll not produced' }
Write-Host "built $($dll.FullName) ($($dll.Length) bytes)"

# ---- 5. package ---------------------------------------------------------------
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Force $dll.FullName $OutDir
Copy-Item -Force LICENSE $OutDir
if (Test-Path ($dll.FullName -replace '\.dll$', '.pdb')) { Copy-Item -Force ($dll.FullName -replace '\.dll$', '.pdb') $OutDir }
New-SourceZip -SrcDir $src -ZipPath (Join-Path $OutDir "ObjectImpactFramework-$UpstreamTag-1.7.104-rebuild-src.zip") -Readme @"
Object Impact Framework $UpstreamTag (commit $upstreamSha) as rebuilt for Skyrim AE 1.7.104 - corresponding source.
Upstream: https://github.com/arachnocid/ObjectImpactFramework (Apache-2.0)
This tree = upstream tag $UpstreamTag + the patches listed in BUILD-NOTES.md (portfile REF/SHA512 bump, three one-line API fixes).
CommonLib is fetched by vcpkg from the overlay port in cmake/ports/commonlibsse-ng:
  https://github.com/alandtse/CommonLibVR commit $CommonLibRef (branch ng, CommonLibSSE-NG 8.0.1, GPL-3.0-or-later with Modding Exception) - unmodified
  https://github.com/ValveSoftware/openvr commit ebdea152f8aac77e9a6db29682b81d762159df7e - unmodified headers
Build: cmake --preset AE ; cmake --build --preset AE   (VS 2022, vcpkg $vcpkgHead, triplet x64-windows-static-md)
"@
Pop-Location

Write-BuildNotes -OutDir $OutDir -ModName 'Object Impact Framework' -Dll 'ObjectImpactFramework.dll' -Facts ([ordered]@{
    'Upstream'          = "https://github.com/arachnocid/ObjectImpactFramework tag ``$UpstreamTag`` (commit ``$upstreamSha``)"
    'Upstream licence'  = 'Apache-2.0 (LICENSE alongside)'
    'Nexus'             = 'https://www.nexusmods.com/skyrimspecialedition/mods/149484'
    'CommonLib'         = "alandtse/CommonLibVR branch ng, commit ``$CommonLibRef`` (CommonLibSSE-NG 8.0.1) via the upstream vcpkg overlay port (REF/SHA512 bumped)"
    'vcpkg'             = "microsoft/vcpkg ``$vcpkgHead``, triplet x64-windows-static-md"
    'Configure'         = '``cmake --preset AE`` (Visual Studio 17 2022, x64, /MD, ENABLE_SKYRIM_AE)'
    'Build'             = '``cmake --build --preset AE`` (Release)'
}) -Patches $patches -Caveats @(
    'Hook offsets inside functions (REL::VariantOffset values) are unchanged from the 1.6.x-era source; the Address Library maps function starts, not the call sites inside them. If an OIF event crashes on 1.7.104 while boot is clean, that is hook drift, not a format-5 problem - report the faulting address.',
    'CIF (Core Impact Framework) is unrelated; keep it installed. Replace only ObjectImpactFramework.dll; keep the existing Data\SKSE\Plugins\ObjectImpactFramework\*.json packs.',
    'Prefer an official arachnocid 1.7.104 build if one appears; then this rebuild is superseded.'
)
Get-ChildItem $OutDir | Format-Table Name, Length
