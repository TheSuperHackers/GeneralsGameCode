<#
.SYNOPSIS
    Builds and runs bench_game_math.c in every requested configuration, then
    removes every build artifact.

.DESCRIPTION
    Compiles bench_game_math.c against the GameMath library once per
    architecture and floating point model, runs it, and leaves only the result
    files in -OutDir (Benchmarks\GameMath\Snapshots by default).

        x86, /fp:precise   ->  bench-win-x86-precise-PC24.txt
                               bench-win-x86-precise-PC53.txt
        x86, /fp:strict    ->  bench-win-x86-strict-PC24.txt
                               bench-win-x86-strict-PC53.txt
        x64, /fp:precise   ->  bench-win-x64-precise.txt
        x64, /fp:strict    ->  bench-win-x64-strict.txt

    With -Reverse every name gains a -rev suffix, so the two measurement orders
    do not overwrite one another.

    On 32-bit x86 each run walks both x87 precision control settings, since the
    game build sets _PC_24 in setFPMode() and the question is what that costs.
    There is no such pair on x64: the precision control belongs to the x87 unit,
    which 64-bit code does not use.

    This is a timing run, so it is worth giving it a quiet machine. The process
    is raised to high priority and each configuration is measured best of three
    rounds inside the program itself.

.PARAMETER Arch
    Architectures to run. Default: both. x86 is what the game builds and the
    only one where _PC_24 and _PC_53 exist; x64 is there because the question
    is which combination of architecture, precision control and argument type
    is quickest, and that needs every cell of the matrix. `-Arch x86` narrows
    it.

.PARAMETER Fp
    Floating point models to run. Default: both precise and strict.

.PARAMETER Reverse
    Measure _PC_53 before _PC_24 instead of the other way round. A timing
    difference that survives both orders is not an artifact of the second run
    starting on a warmer machine. The results are written with a -rev suffix,
    so both orders can sit side by side and be weighed as two configurations.

.PARAMETER BuildDir
    CMake build tree holding the 32-bit GameMath dependency. Its
    _deps\gamemath-src is also the source used to build GameMath for x64.
    Default: <repo>\build\win32

.PARAMETER X64GameMathDir
    Build tree for the 64-bit GameMath. Configured and built on first use and
    reused afterwards.
    Default: <repo>\build\win64-gamemath

.PARAMETER Config
    GameMath build configuration to link against. Default: Release

.PARAMETER OutDir
    Where the result files are written. Default: Benchmarks\GameMath\Snapshots

.PARAMETER RebuildX64
    Configure and build the 64-bit GameMath again even if its library is
    already there.

.PARAMETER KeepExe
    Copy each executable to -OutDir as bench_game_math-<arch>-<fp>.exe instead
    of discarding it.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\run_bench_game_math.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\run_bench_game_math.ps1 -Reverse

.NOTES
    Nanoseconds per call answer only half the question. What the game pays is
    that number times how often it makes the call; weigh_bench.sh combines the
    two using the call profile in callcounts-zh.txt.
#>
[CmdletBinding()]
param(
    [ValidateSet('x86', 'x64')]
    [string[]]$Arch = @('x86', 'x64'),

    [ValidateSet('precise', 'strict')]
    [string[]]$Fp = @('precise', 'strict'),

    [switch]$Reverse,
    [string]$BuildDir,
    [string]$X64GameMathDir,
    [string]$Config = 'Release',
    [string]$OutDir,
    [switch]$RebuildX64,
    [switch]$KeepExe
)

$ErrorActionPreference = 'Stop'

if ($Config -ne 'Release') {
    Write-Warning "timing a $Config library; the numbers are not comparable to the Release ones"
}

$testsDir = Split-Path $PSScriptRoot -Parent
$repoRoot = Split-Path (Split-Path $testsDir -Parent) -Parent

if (-not $BuildDir)       { $BuildDir       = Join-Path $repoRoot 'build\win32' }
if (-not $X64GameMathDir) { $X64GameMathDir = Join-Path $repoRoot 'build\win64-gamemath' }
if (-not $OutDir)         { $OutDir         = Join-Path $testsDir 'Snapshots' }

$source      = Join-Path $testsDir 'Source\bench_game_math.c'
$gamemathSrc = Join-Path $BuildDir '_deps\gamemath-src'
$include     = Join-Path $gamemathSrc 'include'
$gmLibX86    = Join-Path $BuildDir "_deps\gamemath-build\$Config\gm.lib"

# ---------- inputs ----------

if (-not (Test-Path $source)) {
    throw "Source not found: $source"
}
if (-not (Test-Path $include)) {
    throw "GameMath headers not found: $include`nConfigure the build tree first, or pass -BuildDir."
}
if (($Arch -contains 'x86') -and -not (Test-Path $gmLibX86)) {
    throw "32-bit GameMath library not found: $gmLibX86`nBuild the gamemath target first, or pass -BuildDir / -Config."
}
if (($Arch -contains 'x64') -and -not (Test-Path (Join-Path $gamemathSrc 'CMakeLists.txt'))) {
    throw "GameMath sources not found: $gamemathSrc`nThe 64-bit library is built from them; configure the build tree first, or pass -BuildDir."
}
if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

$source      = (Resolve-Path $source).Path
$gamemathSrc = (Resolve-Path $gamemathSrc).Path
$include     = (Resolve-Path $include).Path
$OutDir      = (Resolve-Path $OutDir).Path
if (Test-Path $gmLibX86) { $gmLibX86 = (Resolve-Path $gmLibX86).Path }

# ---------- toolchain ----------

# vcvarsall shells out to vswhere to locate the Windows SDK, but its own
# directory is not necessarily on PATH. Put it there, or the SDK lib paths
# come back incomplete and the link fails on CRT symbols.
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

# Runs a command line inside a fresh developer prompt for one architecture.
# Each call is its own cmd.exe, so the x86 and x64 environments never leak
# into one another.
function Invoke-DevPrompt {
    param(
        [string]$TargetArch,
        [string]$Command,
        [string]$WorkingDirectory,
        [string]$LogPath
    )

    $line = "set ${quote}PATH=$installerDir;%PATH%${quote}" +
            " && cd /d ${quote}$WorkingDirectory${quote}" +
            " && call ${quote}$vcvarsall${quote} $TargetArch >nul" +
            " && $Command"

    & cmd.exe /c "($line) > ${quote}$LogPath${quote} 2>&1"
    return $LASTEXITCODE
}

$modeOrder = if ($Reverse) { 'pc53 pc24' } else { 'pc24 pc53' }

Write-Host "toolchain : $vsRoot"
Write-Host ("matrix    : {0} x {1}" -f ($Arch -join ', '), ($Fp -join ', '))
Write-Host "x87 order : $modeOrder"
Write-Host "output    : $OutDir"
Write-Host ''
Write-Host 'This is a timing run. Close anything else that wants the CPU.'

$work = Join-Path ([System.IO.Path]::GetTempPath()) ('gamemath-bench-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $work -Force | Out-Null

$produced = New-Object System.Collections.Generic.List[string]
$failures = New-Object System.Collections.Generic.List[string]

try {
    # ---------- the 64-bit GameMath ----------
    #
    # Nothing in the repository builds 64-bit - the game is x86 only, and there
    # is no x64 preset - so the library has to be built here, straight from the
    # sources the 32-bit build tree already fetched.

    $gmLibX64 = $null

    if ($Arch -contains 'x64') {
        $cached = Join-Path $X64GameMathDir "$Config\gm.lib"

        if ((Test-Path $cached) -and -not $RebuildX64) {
            $gmLibX64 = (Resolve-Path $cached).Path
            Write-Host "gm.lib x64: $gmLibX64 (reused; -RebuildX64 to build again)"
        }
        else {
            Write-Host ''
            Write-Host "building 64-bit GameMath in $X64GameMathDir ..."

            if (-not (Test-Path $X64GameMathDir)) {
                New-Item -ItemType Directory -Path $X64GameMathDir -Force | Out-Null
            }
            $x64Dir = (Resolve-Path $X64GameMathDir).Path

            $probeLog = Join-Path $work 'ninja-probe.log'
            $hasNinja = (Invoke-DevPrompt -TargetArch 'x64' -Command 'where ninja' -WorkingDirectory $work -LogPath $probeLog) -eq 0

            if ($hasNinja) {
                $generator = "-G ${quote}Ninja Multi-Config${quote}"
                Write-Host 'generator : Ninja Multi-Config'
            }
            else {
                $generator = '-A x64'
                Write-Host 'generator : Visual Studio default, -A x64'
            }

            $runtime  = 'MultiThreaded$<$<CONFIG:Debug>:Debug>DLL'
            $cmakeCfg = "cmake -S ${quote}$gamemathSrc${quote} -B ${quote}$x64Dir${quote} $generator" +
                        " -DGM_ENABLE_TESTS=OFF" +
                        " -DCMAKE_MSVC_RUNTIME_LIBRARY=${quote}$runtime${quote}"
            $cmakeBld = "cmake --build ${quote}$x64Dir${quote} --config $Config"

            $cfgLog = Join-Path $work 'gamemath-x64-configure.log'
            $bldLog = Join-Path $work 'gamemath-x64-build.log'

            $code = Invoke-DevPrompt -TargetArch 'x64' -Command $cmakeCfg -WorkingDirectory $work -LogPath $cfgLog
            if ($code -ne 0) {
                if (Test-Path $cfgLog) { Get-Content $cfgLog | Write-Host }
                throw "Configuring the 64-bit GameMath failed (exit $code)."
            }

            $code = Invoke-DevPrompt -TargetArch 'x64' -Command $cmakeBld -WorkingDirectory $work -LogPath $bldLog
            if ($code -ne 0) {
                if (Test-Path $bldLog) { Get-Content $bldLog | Write-Host }
                throw "Building the 64-bit GameMath failed (exit $code)."
            }

            $lib = @(Get-ChildItem -Path (Join-Path $x64Dir $Config) -Filter 'gm.lib' -File -Recurse -ErrorAction SilentlyContinue) |
                   Select-Object -First 1
            if (-not $lib) {
                throw "The 64-bit GameMath built but gm.lib was not found under $x64Dir."
            }

            $gmLibX64 = $lib.FullName
            Write-Host "gm.lib x64: $gmLibX64"
        }
    }

    # ---------- the matrix ----------

    $crtFlag = '/MD'
    if ($Config -eq 'Debug') { $crtFlag = '/MDd' }

    foreach ($a in $Arch) {
        if ($a -eq 'x86') { $gmLib = $gmLibX86 } else { $gmLib = $gmLibX64 }

        foreach ($f in $Fp) {
            $tag = "$a-$f"

            Write-Host ''
            Write-Host "=== $a, /fp:$f ==="

            $cfgDir = Join-Path $work $tag
            New-Item -ItemType Directory -Path $cfgDir -Force | Out-Null

            $exe      = Join-Path $cfgDir 'bench_game_math.exe'
            $buildLog = Join-Path $cfgDir 'build.log'
            $runLog   = Join-Path $cfgDir 'run.log'

            # /MD is required: gm.lib is built against the dynamic CRT, and the
            # cl default drags in the static one - the mix fails to link on
            # __imp__fesetround and __except_handler4_common. A Debug gm.lib is
            # built against the debug one, so Debug takes /MDd.
            #
            # /wd4005: gmath.h redefines isnan, INFINITY and the comparison
            # macros that math.h already provides.
            $cl = "cl /nologo /O2 /fp:$f $crtFlag /wd4005 ${quote}$source${quote}" +
                  " /I ${quote}$include${quote} ${quote}$gmLib${quote}" +
                  " /Fe:bench_game_math.exe"

            Write-Host 'building...'
            $code = Invoke-DevPrompt -TargetArch $a -Command $cl -WorkingDirectory $cfgDir -LogPath $buildLog

            if ($code -ne 0 -or -not (Test-Path $exe)) {
                if (Test-Path $buildLog) { Get-Content $buildLog | Write-Host }
                $failures.Add("${tag}: build failed (exit $code)")
                continue
            }

            Write-Host 'timing... (a minute or two)'

            # High priority, so an unrelated background task cannot land in the
            # middle of one mode and not the other.
            $proc = Start-Process -FilePath $exe -ArgumentList $modeOrder `
                        -WorkingDirectory $cfgDir -NoNewWindow -PassThru `
                        -RedirectStandardOutput $runLog

            # Reading Handle caches it. Without that ExitCode comes back empty
            # once the process is gone, every run is taken for a failure and
            # its results are thrown away with the work directory. -Wait would
            # cache it too, but the priority has to be raised while the process
            # is still running.
            $null = $proc.Handle

            try { $proc.PriorityClass = 'High' } catch { }
            $proc.WaitForExit()

            if ($proc.ExitCode -ne 0) {
                if (Test-Path $runLog) { Get-Content $runLog | Write-Host }
                $failures.Add("${tag}: run exited with $($proc.ExitCode)")
                continue
            }

            $results = @(Get-ChildItem -Path $cfgDir -Filter 'bench-*.txt' -File)
            if ($results.Count -eq 0) {
                $failures.Add("${tag}: the run produced no result files")
                continue
            }

            foreach ($d in $results) {
                # The program names its file after the configuration and knows
                # nothing about the order it was asked to measure in, so both
                # orders would land on the same name and the second run would
                # quietly overwrite the first. Mark the reversed one here, as
                # the file enters -OutDir; the program stays untouched, and the
                # suffix sits behind the PC24 / PC53 part that weigh_bench.sh
                # pairs on, so the two orders come out as two configurations
                # rather than a broken pair.
                if ($Reverse) { $name = $d.BaseName + '-rev' + $d.Extension }
                else          { $name = $d.Name }

                $dest = Join-Path $OutDir $name
                Move-Item -LiteralPath $d.FullName -Destination $dest -Force
                $produced.Add($name)
                Write-Host ("wrote {0} ({1:N0} bytes)" -f $name, (Get-Item $dest).Length)
            }

            if ($KeepExe) {
                $keptName = "bench_game_math-$tag.exe"
                Copy-Item -LiteralPath $exe -Destination (Join-Path $OutDir $keptName) -Force
                Write-Host "wrote $keptName"
            }
        }
    }
}
finally {
    if (Test-Path $work) {
        Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue
    }

    # A manual build following the header of bench_game_math.c drops these next to the source.
    foreach ($pat in @('*.obj', '*.exp', '*.lib', '*.pdb', '*.ilk', 'bench_game_math*.exe')) {
        Get-ChildItem -Path $testsDir -Filter $pat -File -ErrorAction SilentlyContinue |
            Where-Object { -not ($KeepExe -and $_.Name -like 'bench_game_math-*.exe') } |
            Remove-Item -Force -ErrorAction SilentlyContinue
    }
}

# ---------- report ----------

$stray = @($produced | Where-Object { $_ -like '*fpdefault*' })
if ($stray.Count -gt 0) {
    Write-Host ''
    Write-Warning ("MSVC defined none of _M_FP_STRICT / _M_FP_PRECISE / _M_FP_FAST: " +
                   ($stray -join ', ') + ". The FP_TAG detection in bench_game_math.c needs fixing.")
}

if ($produced.Count -eq 0) {
    Write-Host ''
    Write-Host 'no results were produced'
}

if ($failures.Count -gt 0) {
    Write-Host ''
    Write-Host 'failed configurations:'
    $failures | ForEach-Object { Write-Host "  $_" }
    Write-Host ''
    Write-Host 'no build artifacts left behind'
    exit 1
}

Write-Host ''
Write-Host 'done; no build artifacts left behind'
