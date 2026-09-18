#Requires -Version 5.1
<#
  Factory / Taquitos Windows build for AdventureXP 4.0.0
  Needs: Visual Studio 2022 (MSVC v143), CMake 3.25+, Git, Python 3, Ninja (optional)

  From a normal PowerShell prompt (the script opens the VS x64 environment):

    powershell -ExecutionPolicy Bypass -File scripts\build-vs2022.ps1

  Output:
    build\windows-msvc-release\AdventureXP.dll
    dist\packed\AdventureXP-4.0.0.zip
#>
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

function Find-VsDevCmd {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found. Install Visual Studio 2022 with Desktop C++."
    }
    $install = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $install) {
        throw "VS 2022 with MSVC x64 tools was not found. Taquitos/factory must install the C++ workload."
    }
    $cmd = Join-Path $install "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $cmd)) {
        throw "VsDevCmd.bat missing under $install"
    }
    return $cmd
}

if (-not $env:VCPKG_ROOT) {
    $local = Join-Path $Root ".vcpkg"
    if (-not (Test-Path (Join-Path $local "vcpkg.exe"))) {
        Write-Host "Cloning vcpkg into $local"
        git clone --depth 1 https://github.com/microsoft/vcpkg.git $local
        git -C $local fetch --depth 1 origin ee12231b20c95013c6638d845d04c91559a1d1ff
        git -C $local checkout ee12231b20c95013c6638d845d04c91559a1d1ff
        & (Join-Path $local "bootstrap-vcpkg.bat")
    }
    $env:VCPKG_ROOT = (Resolve-Path $local).Path
}

$vsdev = Find-VsDevCmd
$inner = @"
call `"$vsdev`" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1
cd /d `"$Root`"
cmake --preset windows-msvc-release
if errorlevel 1 exit /b 1
cmake --build --preset windows-msvc-release --parallel
if errorlevel 1 exit /b 1
python scripts\generate_esl.py
if errorlevel 1 exit /b 1
python scripts\pack.py --source
if errorlevel 1 exit /b 1
"@

$bat = Join-Path $env:TEMP "adventurexp-build.bat"
Set-Content -Path $bat -Value $inner -Encoding ASCII
& cmd.exe /c $bat
if ($LASTEXITCODE -ne 0) {
    throw "AdventureXP MSVC build failed with exit $LASTEXITCODE"
}

$dll = Join-Path $Root "build\windows-msvc-release\AdventureXP.dll"
$zip = Join-Path $Root "dist\packed\AdventureXP-4.0.0.zip"
Write-Host "DLL: $dll"
Write-Host "ZIP: $zip"
if (-not (Test-Path $dll)) { throw "AdventureXP.dll was not produced" }
if (-not (Test-Path $zip)) { throw "AdventureXP-4.0.0.zip was not produced" }
Write-Host "AdventureXP 4.0.0 Windows build OK"
