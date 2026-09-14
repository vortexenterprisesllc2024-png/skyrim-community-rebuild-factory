# Shared helpers for the 1.7.104 rebuild jobs (GitHub Actions, windows-latest).
# Dot-source this file:  . "$PSScriptRoot\common.ps1"

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

function Invoke-Checked {
    # Run a native command and fail the script if it returns non-zero.
    param([Parameter(Mandatory)][string]$Exe, [Parameter(ValueFromRemainingArguments)][string[]]$ArgList)
    Write-Host ">> $Exe $($ArgList -join ' ')"
    & $Exe @ArgList
    if ($LASTEXITCODE -ne 0) { throw "$Exe exited with code $LASTEXITCODE" }
}

function Initialize-Vcpkg {
    # Fresh microsoft/vcpkg clone at a known commit. Returns the root path.
    param([string]$Ref = 'master', [string]$Root = "$env:RUNNER_TEMP\vcpkg")
    if (-not (Test-Path "$Root\.git")) {
        Invoke-Checked git clone https://github.com/microsoft/vcpkg.git $Root
    }
    Push-Location $Root
    try {
        if ($Ref -and $Ref -ne 'master') {
            & git checkout --quiet $Ref
            if ($LASTEXITCODE -ne 0) {
                Write-Warning "vcpkg ref $Ref not found; staying on master"
                Invoke-Checked git checkout --quiet master
            }
        }
        $head = (git rev-parse HEAD).Trim()
        Write-Host "vcpkg at $head"
        if (-not (Test-Path "$Root\vcpkg.exe")) {
            Invoke-Checked cmd /c "bootstrap-vcpkg.bat -disableMetrics"
        }
    } finally { Pop-Location }
    $env:VCPKG_ROOT = $Root
    $env:VCPKG_INSTALLATION_ROOT = $Root
    "VCPKG_ROOT=$Root" | Out-File -Append -Encoding utf8 $env:GITHUB_ENV
    "VCPKG_INSTALLATION_ROOT=$Root" | Out-File -Append -Encoding utf8 $env:GITHUB_ENV
    return $head
}

function Get-CommonLib {
    # Clone alandtse/CommonLibVR at an exact commit (branch ng) with submodules.
    param([Parameter(Mandatory)][string]$Ref, [Parameter(Mandatory)][string]$Dest)
    if (-not (Test-Path "$Dest\.git")) {
        Invoke-Checked git clone --no-checkout https://github.com/alandtse/CommonLibVR.git $Dest
    }
    Push-Location $Dest
    try {
        Invoke-Checked git fetch --quiet origin $Ref
        Invoke-Checked git checkout --quiet $Ref
        Invoke-Checked git submodule update --init --recursive --quiet
        $head = (git rev-parse HEAD).Trim()
        $ver = (Select-String -Path CMakeLists.txt -Pattern '^\s*VERSION\s+([0-9.]+)' | Select-Object -First 1).Matches[0].Groups[1].Value
        if (-not (Select-String -Path include\REL\IDDB.h -Pattern 'SSEv5' -Quiet)) {
            throw "CommonLib $head has no Format::SSEv5 - refusing to build a DLL that cannot read Address Library format 5"
        }
        Write-Host "CommonLibSSE-NG $ver at $head (SSEv5 present)"
        return @{ Sha = $head; Version = $ver }
    } finally { Pop-Location }
}

function Get-GitHubArchiveSha512 {
    # SHA512 of https://github.com/<repo>/archive/<sha>.tar.gz, as vcpkg_from_github expects.
    param([Parameter(Mandatory)][string]$Repo, [Parameter(Mandatory)][string]$Sha)
    $url = "https://github.com/$Repo/archive/$Sha.tar.gz"
    $out = Join-Path $env:RUNNER_TEMP ("archive-" + $Sha + ".tar.gz")
    Invoke-WebRequest -Uri $url -OutFile $out -UseBasicParsing
    return (Get-FileHash -Algorithm SHA512 $out).Hash.ToLower()
}

function Edit-File {
    # Literal (not regex) replace that fails loudly if the needle is missing.
    param([Parameter(Mandatory)][string]$Path, [Parameter(Mandatory)][string]$Find, [Parameter(Mandatory)][string]$Replace, [int]$Expect = 1)
    # .NET does not follow Push-Location; give it the full PowerShell-resolved path
    $Path = (Resolve-Path -LiteralPath $Path).ProviderPath
    $text = [IO.File]::ReadAllText($Path)
    if ($text.Contains("`r`n")) {
        # checkout may be CRLF (core.autocrlf); needles are written with plain `n
        if (-not $Find.Contains("`r`n")) { $Find = $Find.Replace("`n", "`r`n") }
        if (-not $Replace.Contains("`r`n")) { $Replace = $Replace.Replace("`n", "`r`n") }
    }
    $count = ([regex]::Matches($text, [regex]::Escape($Find))).Count
    if ($count -ne $Expect) { throw "Edit-File: expected $Expect match(es) of '$Find' in $Path, found $count" }
    [IO.File]::WriteAllText($Path, $text.Replace($Find, $Replace), (New-Object System.Text.UTF8Encoding($false)))
    Write-Host "patched $Path : '$Find' -> '$Replace'"
}

function Write-BuildNotes {
    param(
        [Parameter(Mandatory)][string]$OutDir,
        [Parameter(Mandatory)][string]$ModName,
        [Parameter(Mandatory)][string]$Dll,
        [Parameter(Mandatory)][System.Collections.IDictionary]$Facts,   # [ordered]@{} key -> value
        [string[]]$Patches = @(),
        [string[]]$Caveats = @()
    )
    $dllPath = Join-Path $OutDir $Dll
    $len = (Get-Item $dllPath).Length
    $sha = (Get-FileHash -Algorithm SHA256 $dllPath).Hash.ToLower()
    $lines = @()
    $lines += "# BUILD-NOTES - $ModName - Skyrim AE 1.7.104.0 / SKSE 2.3.1 / Address Library format 5"
    $lines += ""
    $lines += "Community rebuild. Not an official author build. Built on a GitHub Actions ``windows-latest`` runner by the"
    $lines += "skyrim-community-rebuild-factory workflow (run $env:GITHUB_RUN_ID, $(Get-Date -Format 'yyyy-MM-dd HH:mm') UTC)."
    $lines += ""
    $lines += "| Field | Value |"
    $lines += "|---|---|"
    foreach ($k in $Facts.Keys) { $lines += "| $k | $($Facts[$k]) |" }
    $lines += "| DLL | ``$Dll`` |"
    $lines += "| DLL size | $len bytes |"
    $lines += "| DLL SHA256 | ``$sha`` |"
    $lines += "| Compiler | MSVC (Visual Studio 2022, v143) on windows-latest; see workflow log for exact ``cl.exe`` version |"
    $lines += "| Smoke test | NOT TESTED by the workflow - Jo must SKSE-boot to main menu + in-game check before this counts |"
    $lines += ""
    if ($Patches.Count) {
        $lines += "## Source changes made for this rebuild"
        $lines += ""
        foreach ($p in $Patches) { $lines += "- $p" }
        $lines += ""
    }
    $lines += "## Licence note"
    $lines += ""
    $lines += "CommonLibSSE-NG relicensed to GPL-3.0-or-later (with the Modding Exception) on 2026-07-25, before format-5"
    $lines += "support was added. Every DLL linked against it is a GPL-3.0-or-later combined work, so the matching source"
    $lines += "(this plugin's tree as patched + the CommonLib commit above) ships next to the binary as ``*-rebuild-src.zip``."
    $lines += "The plugin's own code keeps its upstream licence (see LICENSE in this folder)."
    $lines += ""
    if ($Caveats.Count) {
        $lines += "## Caveats"
        $lines += ""
        foreach ($c in $Caveats) { $lines += "- $c" }
        $lines += ""
    }
    $lines -join "`n" | Out-File -Encoding utf8 (Join-Path $OutDir 'BUILD-NOTES.md')
    "$sha  $Dll" | Out-File -Encoding ascii (Join-Path $OutDir "$Dll.sha256")
    Write-Host "wrote BUILD-NOTES.md ($Dll $len bytes, sha256 $sha)"
}

function New-SourceZip {
    # GPL corresponding source: the working tree as built (patches applied), minus .git, build output and
    # unmodified bulky third-party checkouts that are pinned by URL + commit in SOURCE-README.txt instead.
    param(
        [Parameter(Mandatory)][string]$SrcDir,
        [Parameter(Mandatory)][string]$ZipPath,
        [Parameter(Mandatory)][string]$Readme,
        [string[]]$ExcludeDirNames = @('.git', 'build', 'buildae', 'vcpkg_installed', 'openvr', 'tests', 'test_consumer', 'test_consumer_live', 'test_package', 'Flash')
    )
    if (Test-Path $ZipPath) { Remove-Item $ZipPath }
    $staging = Join-Path $env:RUNNER_TEMP ('srczip-' + [IO.Path]::GetFileNameWithoutExtension($ZipPath))
    if (Test-Path $staging) { Remove-Item -Recurse -Force $staging }
    New-Item -ItemType Directory -Path $staging | Out-Null
    # robocopy /XD with bare names excludes a directory of that name at any depth (submodule .git dirs included)
    & robocopy $SrcDir $staging /E /XD $ExcludeDirNames /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy failed with $LASTEXITCODE" }
    $Readme + "`n`nDirectories left out of this zip (unmodified, or build output): " + ($ExcludeDirNames -join ', ') + "`n" |
        Out-File -Encoding utf8 (Join-Path $staging 'SOURCE-README.txt')
    Compress-Archive -Path (Join-Path $staging '*') -DestinationPath $ZipPath -CompressionLevel Optimal
    Write-Host "wrote $ZipPath ($((Get-Item $ZipPath).Length) bytes)"
}
