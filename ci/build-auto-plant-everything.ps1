# Auto Plant Everything (Jo Hoeflich's own SKSE plugin) for Skyrim AE 1.7.104 / Address Library format 5.
# Source: sources/auto-plant-everything/. CommonLib comes in the same way the factory's OIF build gets it:
# Object Impact Framework 1.6.8's vcpkg overlay port for commonlibsse-ng, bumped to alandtse/CommonLibVR ng
# (SSEv5-capable) with the identical port fixes from ci/build-oif.ps1.
param(
    [string]$CommonLibRef = 'd13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd',
    [string]$VcpkgRef = 'master',
    [string]$OutDir = "$env:GITHUB_WORKSPACE\releases\auto-plant-everything\1.0.0"
)
. "$PSScriptRoot\common.ps1"

$proj = "$env:GITHUB_WORKSPACE\sources\auto-plant-everything"
$work = "$env:RUNNER_TEMP\src\AutoPlantEverything"
$oif  = "$env:RUNNER_TEMP\src\oif-port"
if (-not (Test-Path "$proj\src\main.cpp")) { throw "plugin source missing: $proj\src\main.cpp" }
$vcpkgHead = Initialize-Vcpkg -Ref $VcpkgRef

# ---- 1. working copy of the plugin -------------------------------------------
Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $work | Out-Null
Copy-Item -Recurse -Force "$proj\*" $work
# overlay ports bypass the registry, so no baseline / version constraint is needed (or wanted)
@'
{
  "name": "autoplanteverything",
  "version-string": "1.0.0",
  "dependencies": [ "commonlibsse-ng", "spdlog" ]
}
'@ | Out-File -Encoding ascii "$work\vcpkg.json"

# ---- 2. commonlibsse-ng overlay port (OIF 1.6.8's, patched exactly as ci/build-oif.ps1 does) ---------
if (-not (Test-Path "$oif\.git")) {
    Invoke-Checked git clone --quiet --depth 1 --branch 1.6.8 https://github.com/arachnocid/ObjectImpactFramework.git $oif
}
New-Item -ItemType Directory -Force -Path "$work\cmake\ports" | Out-Null
Copy-Item -Recurse -Force "$oif\cmake\ports\commonlibsse-ng" "$work\cmake\ports\"
Push-Location $work
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
$portJson = 'cmake\ports\commonlibsse-ng\vcpkg.json'
Edit-File $portJson '"version-semver": "3.7.0"' '"version-semver": "8.0.1"'
Edit-File $portJson "`"dependencies`": [`n    {`n      `"name`": `"vcpkg-cmake-config`",`n      `"host`": true`n    }," "`"dependencies`": [`n    {`n      `"name`": `"vcpkg-cmake`",`n      `"host`": true`n    },`n    {`n      `"name`": `"vcpkg-cmake-config`",`n      `"host`": true`n    },"
Edit-File $portJson "`"default-features`": [`n    `"tests`"`n  ]" "`"default-features`": []"
Get-Content $port

# ---- 3. configure + build ------------------------------------------------------
$triplets = New-ReleaseOnlyTriplet -Name 'x64-windows-static-md'
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
try {
    Invoke-Checked cmake '-S' . '-B' build '-G' 'Visual Studio 17 2022' '-A' 'x64' `
        "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" `
        "-DVCPKG_OVERLAY_PORTS=$work\cmake\ports" `
        "-DVCPKG_OVERLAY_TRIPLETS=$triplets" `
        '-DVCPKG_TARGET_TRIPLET=x64-windows-static-md' `
        '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL'
} catch {
    Show-VcpkgFailureLogs -Port 'commonlibsse-ng'
    throw
}
Invoke-Checked cmake --build build --config Release
$dll = Get-ChildItem -Recurse -Filter AutoPlantEverything.dll build | Select-Object -First 1
if (-not $dll) { throw 'AutoPlantEverything.dll not produced' }
if ($dll.Length -lt 50000) { throw "AutoPlantEverything.dll is only $($dll.Length) bytes - not a real build" }
Write-Host "built $($dll.FullName) ($($dll.Length) bytes)"

# ---- 4. export check -----------------------------------------------------------
@'
import sys, pefile
pe = pefile.PE(sys.argv[1])
assert pe.FILE_HEADER.Machine == 0x8664, 'not x64'
names = {e.name.decode() for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
assert {'SKSEPlugin_Load', 'SKSEPlugin_Version'} <= names, names
print('exports OK:', sorted(names))
'@ | Out-File -Encoding ascii "$work\verify.py"
Invoke-Checked python '-m' pip install --quiet pefile
Invoke-Checked python "$work\verify.py" $dll.FullName

# ---- 5. package: DLL + Vortex/Nexus zip ---------------------------------------
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$dist = "$env:RUNNER_TEMP\ape-dist"
Remove-Item -Recurse -Force $dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path "$dist\SKSE\Plugins", "$dist\fomod" | Out-Null
Copy-Item $dll.FullName "$dist\SKSE\Plugins\"
Copy-Item "$work\package\SKSE\Plugins\AutoPlantEverything.ini" "$dist\SKSE\Plugins\"
Copy-Item "$work\package\fomod\info.xml" "$dist\fomod\"
Compress-Archive -Path "$dist\*" -DestinationPath "$OutDir\AutoPlantEverything-1.0.0.zip" -Force
Copy-Item -Force $dll.FullName $OutDir
Pop-Location

Write-BuildNotes -OutDir $OutDir -ModName 'Auto Plant Everything' -Dll 'AutoPlantEverything.dll' -Facts ([ordered]@{
    'Source'    = 'sources/auto-plant-everything (Jo Hoeflich, original work)'
    'CommonLib' = "alandtse/CommonLibVR branch ng, commit ``$CommonLibRef`` (CommonLibSSE-NG 8.0.1) via OIF 1.6.8's overlay port, patched as in ci/build-oif.ps1"
    'vcpkg'     = "microsoft/vcpkg ``$vcpkgHead``, triplet x64-windows-static-md (release only)"
    'Build'     = 'Visual Studio 17 2022, x64, Release, /MD'
}) -Patches @() -Caveats @('Own plugin, not a rebuild of someone else''s mod.')
Get-ChildItem $OutDir | Format-Table Name, Length
