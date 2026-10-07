<#
.SYNOPSIS
    The whole GameMath refresh in one command: rebuilds the 32-bit library,
    then writes the verification dumps and both benchmark passes, compares the
    dumps against the macOS one and weighs the timings by the call profile.

.DESCRIPTION
    The sequence to run after the pin in cmake/gamemath.cmake moves, as a
    single script:

        cmake --preset win32
        cmake --build build\win32 --config Release --target gamemath
        run_verify_game_math.ps1 -RebuildX64
        run_bench_game_math.ps1
        run_bench_game_math.ps1 -Reverse
        compare_math.ps1
        weigh_bench.ps1

    The two cmake steps drive the win32 Ninja tree, which needs the MSVC
    environment. This script calls vcvarsall.bat itself, the same way the
    verify and benchmark scripts do, so an ordinary PowerShell is enough.
    Without that environment cl.exe is still found - the CMake cache holds its
    absolute path - but INCLUDE is unset and every file of the library fails on
    "C1083: Cannot open include file: 'stdint.h'".

    The library build gates the rest: if it fails the run stops there, because
    the dumps would otherwise quietly describe the old revision, which is the
    failure this sequence exists to prevent. The benchmark passes run only
    after verify, which rebuilds the 64-bit library; the comparison only after
    the dumps were written; and the weighting only after both benchmark passes
    and the comparison, since each would otherwise describe old files.
    Whatever failed is listed at the end and the script exits non-zero.

    The comparison and the weighting take the macOS results from the
    repository; -Pull brings in the latest ones.

    Nothing is left behind beyond the dump files, the timings and what is
    computed from them: the child scripts clean up after themselves, and this
    one adds no artifacts of its own.

.PARAMETER Arch
    Architectures to dump and benchmark, passed to both child scripts.
    Default: both x86 and x64. With x64 alone the tree is still configured,
    since the 64-bit library is built from its GameMath sources, but the 32-bit
    library is not rebuilt, since nothing would link against it.

.PARAMETER Fp
    Floating point models to run, passed to both child scripts.
    Default: both precise and strict.

.PARAMETER Pull
    Run "git pull" in the repository first. Off by default; moving the working
    tree is left to the caller unless asked for.

.PARAMETER SkipBuild
    Skip the two cmake steps and go straight to the dumps. Only correct when
    build\win32 already holds a gm.lib from the current pin.

.PARAMETER Preset
    CMake configure preset for the 32-bit tree. Default: win32

.PARAMETER BuildDir
    The build tree that preset produces, and the one the child scripts read.
    Default: <repo>\build\win32

.PARAMETER Config
    GameMath build configuration. Default: Release

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\run_all_game_math.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\run_all_game_math.ps1 -Pull -Arch x86

.NOTES
    The benchmark halves measure time, so the machine should be otherwise idle
    for the second half of this run. The verification dumps do not care.
#>
[CmdletBinding()]
param(
    [ValidateSet('x86', 'x64')]
    [string[]]$Arch = @('x86', 'x64'),

    [ValidateSet('precise', 'strict')]
    [string[]]$Fp = @('precise', 'strict'),

    [switch]$Pull,
    [switch]$SkipBuild,
    [string]$Preset = 'win32',
    [string]$BuildDir,
    [string]$Config = 'Release'
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path $PSScriptRoot -Parent
$testsDir = Join-Path $repoRoot 'Tests\GameMath'
$benchDir = Join-Path $repoRoot 'Benchmarks\GameMath'

if (-not $BuildDir) { $BuildDir = Join-Path $repoRoot "build\$Preset" }

$verifyScript = Join-Path $testsDir 'Scripts\run_verify_game_math.ps1'
$benchScript   = Join-Path $benchDir 'Scripts\run_bench_game_math.ps1'
$compareScript = Join-Path $testsDir 'Scripts\compare_math.ps1'
$weighScript   = Join-Path $benchDir 'Scripts\weigh_bench.ps1'

foreach ($s in @($verifyScript, $benchScript, $compareScript, $weighScript)) {
    if (-not (Test-Path $s)) { throw "Script not found: $s" }
}

# ---------- toolchain ----------

# vcvarsall shells out to vswhere to locate the Windows SDK, but its own
# directory is not necessarily on PATH. Put it there, or the SDK paths come
# back incomplete.
$installerDir = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
$vswhere      = Join-Path $installerDir 'vswhere.exe'

$vsRoot = $null
if (Test-Path $vswhere) {
    $found = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($found) { $vsRoot = ([string]$found).Trim() }
}

if (-not $vsRoot) {
    foreach ($base in @($env:ProgramFiles, ${env:ProgramFiles(x86)})) {
        if (-not $base) { continue }
        foreach ($year in @('2022', '2019')) {
            foreach ($ed in @('Enterprise', 'Professional', 'Community', 'BuildTools')) {
                $probe = Join-Path $base "Microsoft Visual Studio\$year\$ed"
                if (Test-Path (Join-Path $probe 'VC\Auxiliary\Build\vcvarsall.bat')) {
                    $vsRoot = $probe
                    break
                }
            }
            if ($vsRoot) { break }
        }
        if ($vsRoot) { break }
    }
}

if (-not $vsRoot) {
    throw 'No Visual Studio installation with the C++ toolchain was found.'
}

$vcvarsall = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat'
if (-not (Test-Path $vcvarsall)) {
    throw "vcvarsall.bat not found under: $vsRoot"
}

$quote = [char]34

# Wraps one command line in a fresh x86 developer prompt. The caller runs it,
# rather than a function running it and swallowing the compiler output into a
# return value; the environment dies with the cmd.exe that carried it.
function Get-DevPromptLine {
    param(
        [string]$Command
    )

    return "set ${quote}PATH=$installerDir;%PATH%${quote}" +
           " && cd /d ${quote}$repoRoot${quote}" +
           " && call ${quote}$vcvarsall${quote} x86 >nul" +
           " && $Command"
}

Write-Host "toolchain : $vsRoot"
Write-Host "repository: $repoRoot"
Write-Host "build tree: $BuildDir"
Write-Host ("matrix    : {0} x {1}" -f ($Arch -join ', '), ($Fp -join ', '))

$failures = New-Object System.Collections.Generic.List[string]

# ---------- the working tree ----------

if ($Pull) {
    Write-Host ''
    Write-Host '=== git pull ==='
    & git -C $repoRoot pull
    if ($LASTEXITCODE -ne 0) {
        throw 'git pull failed; nothing was rebuilt.'
    }
}

# ---------- the 32-bit library ----------

# The one step everything downstream depends on. The dumps link whatever
# build\win32 holds, so a stale or missing gm.lib here means six files that
# describe the wrong revision - better no dumps than wrong ones.

if (-not $SkipBuild) {
    Write-Host ''
    Write-Host "=== cmake --preset $Preset ==="
    & cmd.exe /c (Get-DevPromptLine "cmake --preset $Preset")
    if ($LASTEXITCODE -ne 0) {
        throw "cmake --preset $Preset failed with exit code $LASTEXITCODE; nothing else was run."
    }
}

if ($SkipBuild) {
    Write-Host ''
    Write-Host '=== gamemath (x86): skipped by -SkipBuild ==='
}
elseif ($Arch -notcontains 'x86') {
    Write-Host ''
    Write-Host '=== gamemath (x86): skipped, no x86 in the matrix ==='
}
else {
    Write-Host ''
    Write-Host '=== gamemath (x86) ==='
    & cmd.exe /c (Get-DevPromptLine "cmake --build ${quote}$BuildDir${quote} --config $Config --target gamemath")
    if ($LASTEXITCODE -ne 0) {
        throw "Building gamemath failed with exit code $LASTEXITCODE; the dumps were not run, so nothing in Tests\GameMath\ or Benchmarks\GameMath\ was overwritten with numbers from the old library."
    }

    $gmLib = Join-Path $BuildDir "_deps\gamemath-build\$Config\gm.lib"
    if (Test-Path $gmLib) {
        $item = Get-Item $gmLib
        Write-Host ("gm.lib    : {0:N0} bytes, {1:yyyy-MM-dd HH:mm}" -f $item.Length, $item.LastWriteTime)
    }
}

# ---------- the pin ----------

# FetchContent does not always move the sources when the tag changes, and a
# library built from the old checkout looks exactly like a fix that did nothing.

$pinFile   = Join-Path $repoRoot 'cmake\gamemath.cmake'
$pinnedTag = ([regex]::Match((Get-Content $pinFile -Raw), 'GIT_TAG\s+(\S+)')).Groups[1].Value
$sourceDir = Join-Path $BuildDir '_deps\gamemath-src'
$sourceRev = [string](& git -C $sourceDir rev-parse HEAD)

if ($LASTEXITCODE -ne 0) {
    throw "Cannot read the GameMath revision in $sourceDir; nothing else was run."
}

$sourceRev = $sourceRev.Trim()
if ($sourceRev -ne $pinnedTag) {
    throw "GameMath sources are at $sourceRev, the pin is $pinnedTag; nothing else was run."
}

Write-Host "gamemath  : $sourceRev"

# ---------- the dumps ----------

# The children run in this process. Through "powershell -File" an array
# argument arrives as separate strings: -Arch keeps only its first value and
# the rest fall through to positional parameters such as -OutDir.

$common = @{ Arch = $Arch; Fp = $Fp; Config = $Config; BuildDir = $BuildDir }

$runs = @(
    @{ Name = 'verify';         Script = $verifyScript; Extra = @{ RebuildX64 = $true } },
    @{ Name = 'bench';          Script = $benchScript;  Extra = @{} },
    @{ Name = 'bench -Reverse'; Script = $benchScript;  Extra = @{ Reverse = $true } }
)

foreach ($run in $runs) {
    Write-Host ''
    Write-Host ("=== {0} ===" -f $run.Name)

    $verifyFailed = @($failures | Where-Object { $_.StartsWith('verify ') }).Count -gt 0
    if ($run.Name -ne 'verify' -and $verifyFailed) {
        Write-Host 'skipped: verify failed, so the 64-bit library may not be current'
        continue
    }

    $params = $common + $run.Extra
    $global:LASTEXITCODE = 0
    try {
        & $run.Script @params
    }
    catch {
        Write-Host $_
        $failures.Add(("{0} ({1})" -f $run.Name, $_.Exception.Message))
        continue
    }

    if ($LASTEXITCODE -ne 0) {
        $failures.Add(("{0} (exit code {1})" -f $run.Name, $LASTEXITCODE))
    }
}

# ---------- the comparison ----------

Write-Host ''
Write-Host '=== compare dumps ==='

$verifyFailed = @($failures | Where-Object { $_.StartsWith('verify ') }).Count -gt 0
if ($verifyFailed) {
    Write-Host 'skipped: verify failed, so the dumps are not current'
}
else {
    try {
        & $compareScript
    }
    catch {
        Write-Host $_
        $failures.Add(("compare ({0})" -f $_.Exception.Message))
    }
}

# ---------- the weighting ----------

Write-Host ''
Write-Host '=== weigh benchmarks ==='

$benchFailed   = @($failures | Where-Object { $_.StartsWith('bench ') }).Count -gt 0
$compareFailed = @($failures | Where-Object { $_.StartsWith('verify ') -or $_.StartsWith('compare ') }).Count -gt 0
if ($benchFailed) {
    Write-Host 'skipped: a benchmark pass failed, so the timings are not current'
}
elseif ($compareFailed) {
    Write-Host 'skipped: verify or the comparison failed, so math-summary.txt is not current'
}
else {
    try {
        & $weighScript
    }
    catch {
        Write-Host $_
        $failures.Add(("weigh ({0})" -f $_.Exception.Message))
    }
}

# ---------- report ----------

Write-Host ''
if ($failures.Count -gt 0) {
    Write-Host 'failed runs:'
    $failures | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host 'done; dumps and their comparison are in Tests\GameMath\Snapshots, timings and their weighting in Benchmarks\GameMath\Snapshots'
