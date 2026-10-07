<#
.SYNOPSIS
    Weights the benchmark timings by how often the game actually calls each
    function, and writes bench-weighted.txt.

.DESCRIPTION
    The Windows twin of weigh_bench.sh. It writes the same bench-weighted.txt.

    Inputs, read from Benchmarks\GameMath\Snapshots:

        callcounts-zh.txt   calls per frame, one column per load profile
        bench-*.txt         nanoseconds per call, one file per configuration

    and from Tests\GameMath\Snapshots:

        math-summary.txt    rows that differ from macOS, per configuration

    Results from other machines come from the repository, so pull first if
    they were refreshed elsewhere.

    Windows writes CRLF, so line endings are normalised on the way in. The file
    is written with LF and no byte order mark. Numbers are rounded the way the
    C printf of awk rounds them, from the exact binary value, so both scripts
    produce the same output from the same inputs.

.PARAMETER BenchDir
    Directory holding the inputs, also where bench-weighted.txt is written.
    Default: Benchmarks\GameMath\Snapshots

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\weigh_bench.ps1
#>
[CmdletBinding()]
param(
    [string]$BenchDir
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Numerics

$benchmarkDir = Split-Path $PSScriptRoot -Parent

if (-not $BenchDir) { $BenchDir = Join-Path $benchmarkDir 'Snapshots' }

$BenchDir = (Resolve-Path $BenchDir).Path

$profileName = 'callcounts-zh.txt'
$outName     = 'bench-weighted.txt'
$summaryName = '../../../Tests/GameMath/Snapshots/math-summary.txt'

# ---------- C printf formatting ----------

function Format-Fixed {
    param([double]$Value, [int]$Decimals)

    $bits     = [BitConverter]::DoubleToInt64Bits($Value)
    $negative = $bits -lt 0
    $exponent = [int](($bits -shr 52) -band 0x7FF)
    $mantissa = $bits -band 0xFFFFFFFFFFFFF
    if ($exponent -eq 0) { $exponent = 1 } else { $mantissa = $mantissa -bor 0x10000000000000 }
    $exponent -= 1075

    $numerator   = [System.Numerics.BigInteger]$mantissa * [System.Numerics.BigInteger]::Pow(10, $Decimals)
    $denominator = [System.Numerics.BigInteger]::One
    if ($exponent -ge 0) {
        $numerator = $numerator * [System.Numerics.BigInteger]::Pow(2, $exponent)
    }
    else {
        $denominator = [System.Numerics.BigInteger]::Pow(2, -$exponent)
    }

    $remainder = [System.Numerics.BigInteger]::Zero
    $quotient  = [System.Numerics.BigInteger]::DivRem($numerator, $denominator, [ref]$remainder)
    $half      = ($remainder * 2).CompareTo($denominator)
    if ($half -gt 0 -or ($half -eq 0 -and -not $quotient.IsEven)) {
        $quotient = $quotient + 1
    }

    $digits = $quotient.ToString().PadLeft($Decimals + 1, '0')
    $text   = $digits
    if ($Decimals -gt 0) {
        $text = $digits.Substring(0, $digits.Length - $Decimals) + '.' + $digits.Substring($digits.Length - $Decimals)
    }
    if ($negative) { return '-' + $text }
    return $text
}

function Format-Float {
    param([double]$Value, [int]$Width, [int]$Decimals, [switch]$Plus)

    $text = Format-Fixed $Value $Decimals
    if ($Plus -and -not $text.StartsWith('-')) { $text = '+' + $text }
    return $text.PadLeft($Width)
}

function Format-Integer {
    param([double]$Value, [int]$Width = 0)

    return ([long][Math]::Truncate($Value)).ToString().PadLeft($Width)
}

function Get-Bar {
    param([int]$Width)

    return '-' * $Width
}

# ---------- inputs ----------

function Read-Lines {
    param([string]$Name)

    $text  = [IO.File]::ReadAllText((Join-Path $BenchDir $Name))
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]$text.Replace("`r", '').Split("`n"))
    if ($lines.Count -gt 0 -and $lines[$lines.Count - 1] -eq '') {
        $lines.RemoveAt($lines.Count - 1)
    }
    return ,$lines.ToArray()
}

function New-Table {
    return New-Object System.Collections.Hashtable ([StringComparer]::Ordinal)
}

function Split-Fields {
    param([string]$Line)

    return ,[string[]]@(-split $Line | Where-Object { $_ -ne '' })
}

function Add-ProfileRow {
    param($Result, [string[]]$Fields)

    $name = $Fields[0]
    $Result.Functions.Add($name)
    $Result.Row[$name]    = $Fields[1]
    $Result.Column[$name] = $Fields[2]
    for ($p = 1; $p -le $Result.Names.Count; $p++) {
        $calls = 0.0
        if (2 + $p -lt $Fields.Count) { $calls = [double]$Fields[2 + $p] }
        $Result.Calls["$name|$p"] = $calls
    }
}

function Read-CallProfile {
    $result = @{
        Names     = New-Object System.Collections.Generic.List[string]
        Frames    = New-Object System.Collections.Generic.List[double]
        Functions = New-Object System.Collections.Generic.List[string]
        Row       = New-Table
        Column    = New-Table
        Calls     = New-Table
    }

    foreach ($line in (Read-Lines $profileName)) {
        $fields = Split-Fields $line
        if ($fields.Count -eq 0 -or $fields[0].StartsWith('#')) { continue }

        if ($fields[0] -ceq 'profiles') {
            for ($i = 1; $i -lt $fields.Count; $i++) { $result.Names.Add($fields[$i]) }
            continue
        }
        if ($fields[0] -ceq 'frames') {
            for ($i = 1; $i -lt $fields.Count; $i++) { $result.Frames.Add([double]$fields[$i]) }
            continue
        }
        Add-ProfileRow $result $fields
    }
    return $result
}

function Read-Mismatch {
    $result  = New-Table
    $columns = @{}
    foreach ($line in (Read-Lines $summaryName)) {
        $fields = Split-Fields $line
        if ($line.StartsWith('row kind')) {
            for ($i = 3; $i -lt $fields.Count; $i++) { $columns[$i] = $fields[$i] }
            continue
        }
        if (-not ($line.StartsWith('through double ') -or $line.StartsWith('plain float '))) { continue }

        $kind = $fields[0] + ' ' + $fields[1]
        for ($i = 3; $i -lt $fields.Count; $i++) { $result["$kind|$($columns[$i])"] = [int]$fields[$i] }
    }
    return $result
}

function Get-Label {
    param([string]$Name)

    $label = (($Name -replace '^bench-', '') -replace '\.txt$', '') -replace '^win-', ''
    return ([regex]'precise').Replace($label, 'prec', 1)
}

function Get-ColumnKey {
    param([string]$Config)

    $precision = '0'
    if ($Config -cmatch 'PC24')     { $precision = '1' }
    elseif ($Config -cmatch 'PC53') { $precision = '2' }
    return $Config.Substring(0, [Math]::Min(3, $Config.Length)) + $precision + $Config
}

function Add-BenchmarkRows {
    param($Result, [string]$Name)

    $label = Get-Label $Name
    foreach ($line in (Read-Lines $Name)) {
        $fields = Split-Fields $line
        if ($fields.Count -ne 7 -or $fields[1] -notmatch '^[0-9.]+$') { continue }

        if (-not $Result.Seen.ContainsKey($label)) {
            $Result.Seen[$label] = $true
            $Result.Configs.Add($label)
        }
        $key = "$label|$($fields[0])"
        $Result.GmDouble[$key]  = [double]$fields[1]
        $Result.SysDouble[$key] = [double]$fields[2]
        $Result.GmFloat[$key]   = [double]$fields[4]
        $Result.SysFloat[$key]  = [double]$fields[5]
    }
}

function Read-Benchmarks {
    $names = [string[]]@(Get-ChildItem -Path $BenchDir -Filter 'bench-*.txt' | ForEach-Object { $_.Name } | Where-Object { $_ -cne $outName })
    if ($names.Count -eq 0) {
        throw 'no benchmark results found (bench-*.txt)'
    }
    [Array]::Sort($names, [StringComparer]::Ordinal)

    $result = @{
        Configs   = New-Object System.Collections.Generic.List[string]
        Seen      = New-Table
        GmDouble  = New-Table
        GmFloat   = New-Table
        SysDouble = New-Table
        SysFloat  = New-Table
    }
    foreach ($name in $names) { Add-BenchmarkRows $result $name }

    $result.Configs.Sort([Comparison[string]] {
        param($first, $second)
        [string]::CompareOrdinal((Get-ColumnKey $first), (Get-ColumnKey $second))
    })
    return $result
}

# ---------- costs ----------

# The entry point the game reaches the function through, or another one with
# -Want: gm_sqrtf and gm_sqrt both become sqrt.d under "all double" and sqrt.f
# under "all float"; the call counts never change, only the way in.
function Get-Ns {
    param([string]$Config, [string]$Function, [string]$Want)

    if (-not $Want) { $Want = $callProfile.Column[$Function] }
    $key = "$Config|$($callProfile.Row[$Function])"
    if ($Want -ceq 'd') { return [double]$benches.GmDouble[$key] }
    return [double]$benches.GmFloat[$key]
}

function Get-SysNs {
    param([string]$Config, [string]$Function, [string]$Want)

    if (-not $Want) { $Want = $callProfile.Column[$Function] }
    $key = "$Config|$($callProfile.Row[$Function])"
    if ($Want -ceq 'd') { return [double]$benches.SysDouble[$key] }
    return [double]$benches.SysFloat[$key]
}

function Get-Rate {
    param([string]$Function, [int]$ProfileIndex)

    return $callProfile.Calls["$Function|$ProfileIndex"] / $callProfile.Frames[$ProfileIndex - 1]
}

function Get-FrameCost {
    param([string]$Config, [int]$ProfileIndex)

    $sum = 0.0
    foreach ($f in $callProfile.Functions) { $sum += (Get-Rate $f $ProfileIndex) * (Get-Ns $Config $f) / 1e6 }
    return $sum
}

function Get-DeltaPercent {
    param([double]$First, [double]$Second)

    if ($Second -gt 0) { return ($First - $Second) / $Second * 100 }
    return 0.0
}

# The answer is read off the busiest profile: the plateau where the highest
# call volume was seen.
function Get-BusiestProfile {
    $decide  = 1
    $bestVol = 0.0
    for ($p = 1; $p -le $callProfile.Names.Count; $p++) {
        $vol = 0.0
        foreach ($f in $callProfile.Functions) { $vol += Get-Rate $f $p }
        if ($vol -gt $bestVol) {
            $bestVol = $vol
            $decide  = $p
        }
    }
    return $decide
}

# ---------- output ----------

function Add-Line {
    param([string]$Text = '')

    [void]$out.Append($Text).Append("`n")
}

function Get-Bench {
    param([string]$Function)

    return $callProfile.Row[$Function] + '.' + $callProfile.Column[$Function]
}

function Get-ProfileTitle {
    param([int]$ProfileIndex)

    return '{0} profile ({1} frames)' -f $callProfile.Names[$ProfileIndex - 1], (Format-Integer $callProfile.Frames[$ProfileIndex - 1])
}

function Add-Header {
    $lines = @(
        'GameMath cost per logic frame'
        ('generated {0} UTC' -f (Get-Date).ToUniversalTime().ToString('yyyy-MM-dd HH:mm'))
        ''
        'Nanoseconds per call from bench-*.txt, multiplied by calls per frame'
        "from ${profileName}:"
        ''
        '  ms per frame = calls per frame * ns per call / 1e6'
        ''
        'The call profile is built without reference to any timing, so the same'
        'profile can be laid against any implementation. This file is the step'
        'where the two meet, and the only place milliseconds appear.'
        ''
        'Only the GameMath columns of the benchmark are used; the system libm'
        'ones are carried along for reference.'
        ''
        'This is an upper bound. The benchmark calls each function in a tight'
        'loop with a hot cache and a predicted branch.'
        ''
        'Next to each cost in the matrix: how many rows of that route differ'
        'from macOS in Tests/GameMath/Snapshots/math-summary.txt, or ok if none do.'
        ''
    )
    foreach ($line in $lines) { Add-Line $line }
}

# ---------- calls and nanoseconds ----------

function Add-CallsSection {
    $names = $callProfile.Names

    Add-Line '---- calls per logic frame ----'
    Add-Line
    $line = '{0,-12} {1,-8}' -f 'gm function', 'bench'
    foreach ($name in $names) { $line += ' {0,16} {1,12}' -f "$name calls", 'per frame' }
    Add-Line $line
    $line = '{0,-12} {1,-8}' -f '------------', '--------'
    foreach ($name in $names) { $line += ' {0,16} {1,12}' -f (Get-Bar 16), (Get-Bar 12) }
    Add-Line $line

    foreach ($f in $callProfile.Functions) {
        $line = '{0,-12} {1,-8}' -f $f, (Get-Bench $f)
        for ($p = 1; $p -le $names.Count; $p++) {
            $calls = $callProfile.Calls["$f|$p"]
            $line += ' ' + (Format-Integer $calls 16) + ' ' + (Format-Float (Get-Rate $f $p) 12 4)
        }
        Add-Line $line
    }

    $line = '{0,-12} {1,-8}' -f 'frames', ''
    for ($p = 1; $p -le $names.Count; $p++) {
        $line += ' ' + (Format-Integer $callProfile.Frames[$p - 1] 16) + (' {0,12}' -f '')
    }
    Add-Line $line
}

function Add-NanosecondsSection {
    $configs = $benches.Configs

    Add-Line
    Add-Line '---- nanoseconds per call ----'
    Add-Line
    $line = '{0,-12} {1,-8}' -f 'gm function', 'bench'
    foreach ($config in $configs) { $line += ' {0,16}' -f $config }
    Add-Line ($line + (' {0,10}' -f 'sys libm'))
    $line = '{0,-12} {1,-8}' -f '------------', '--------'
    foreach ($config in $configs) { $line += ' {0,16}' -f (Get-Bar 16) }
    Add-Line ($line + (' {0,10}' -f (Get-Bar 10)))

    foreach ($f in $callProfile.Functions) {
        $line = '{0,-12} {1,-8}' -f $f, (Get-Bench $f)
        foreach ($config in $configs) { $line += ' ' + (Format-Float (Get-Ns $config $f) 16 1) }
        Add-Line ($line + ' ' + (Format-Float (Get-SysNs $configs[0] $f) 10 1))
    }
}

# ---------- the superposition matrix ----------
#
# Every call in the profile costed three ways: through the double entry point,
# through the float one, and as the game reaches them today.

function Add-MatrixLegend {
    $lines = @(
        'ms per logic frame. Same calls, same counts, different way in.'
        ''
        'Next to each cost: rows of that route that differ from macOS in'
        'Tests/GameMath/Snapshots/math-summary.txt, or ok if none do.'
        ''
        '  as the game  through double and plain float rows'
        '  all double   through double rows'
        '  all float    plain float rows'
        ''
    )
    foreach ($line in $lines) { Add-Line $line }
}

function Add-MatrixRow {
    param([string]$Config, [int]$ProfileIndex)

    $mix = 0.0; $allDouble = 0.0; $allFloat = 0.0
    foreach ($f in $callProfile.Functions) {
        $rate = Get-Rate $f $ProfileIndex
        $mix       += $rate * (Get-Ns $Config $f) / 1e6
        $allDouble += $rate * (Get-Ns $Config $f 'd') / 1e6
        $allFloat  += $rate * (Get-Ns $Config $f 'f') / 1e6
    }

    $differingDouble = Get-Differing $Config 'through double'
    $differingFloat  = Get-Differing $Config 'plain float'
    $differingMix    = -1
    if ($differingDouble -ge 0 -and $differingFloat -ge 0) { $differingMix = $differingDouble + $differingFloat }

    $best = 'none'; $low = 0.0
    if ($differingMix -eq 0) { $best = 'as the game'; $low = $mix }
    if ($differingDouble -eq 0 -and ($best -ceq 'none' -or $allDouble -lt $low)) { $best = 'all double'; $low = $allDouble }
    if ($differingFloat -eq 0 -and ($best -ceq 'none' -or $allFloat -lt $low)) { $best = 'all float'; $low = $allFloat }

    Add-Line ('{0,-22}  {1} {2,-7}  {3} {4,-7}  {5} {6,-7}  {7}' -f $Config,
        (Format-Float $mix 9 4), (Get-Verdict $differingMix), (Format-Float $allDouble 9 4), (Get-Verdict $differingDouble),
        (Format-Float $allFloat 9 4), (Get-Verdict $differingFloat), $best)
}

function Get-Differing {
    param([string]$Config, [string]$Kind)

    $name = $Config -replace '-rev$', ''
    if ($name.StartsWith('mac-')) { return 0 }
    $key = "$Kind|$name"
    if (-not $mismatch.ContainsKey($key)) { return -1 }
    return $mismatch[$key]
}

function Get-Verdict {
    param([int]$Count)

    if ($Count -lt 0)  { return '?' }
    if ($Count -eq 0)  { return 'ok' }
    return "($Count)"
}

function Add-MatrixSection {
    for ($p = 1; $p -le $callProfile.Names.Count; $p++) {
        Add-Line
        Add-Line ('---- superposition matrix, {0} ----' -f (Get-ProfileTitle $p))
        Add-Line
        Add-MatrixLegend

        Add-Line ('{0,-22}  {1,17}  {2,17}  {3,17}  {4}' -f 'configuration', 'as the game', 'all double', 'all float', 'fastest valid')
        Add-Line ('{0,-22}  {1,17}  {2,17}  {3,17}  {4}' -f (Get-Bar 22), (Get-Bar 17), (Get-Bar 17), (Get-Bar 17), (Get-Bar 13))
        foreach ($config in $benches.Configs) { Add-MatrixRow $config $p }
    }
}

# ---------- GameMath against the platform library ----------

function Add-SystemRow {
    param([string]$Config, [int]$ProfileIndex)

    $gmMix = 0.0; $sysMix = 0.0; $gmFloat = 0.0; $sysFloat = 0.0
    foreach ($f in $callProfile.Functions) {
        $rate = Get-Rate $f $ProfileIndex
        $gmMix    += $rate * (Get-Ns $Config $f) / 1e6
        $sysMix   += $rate * (Get-SysNs $Config $f) / 1e6
        $gmFloat  += $rate * (Get-Ns $Config $f 'f') / 1e6
        $sysFloat += $rate * (Get-SysNs $Config $f 'f') / 1e6
    }

    $ratioMix = 0.0; $ratioFloat = 0.0
    if ($sysMix -gt 0)   { $ratioMix = $gmMix / $sysMix }
    if ($sysFloat -gt 0) { $ratioFloat = $gmFloat / $sysFloat }

    Add-Line ('{0,-22}  {1} {2}  {3} {4}   x{5} / x{6}' -f $Config,
        (Format-Float $gmMix 8 4), (Format-Float $sysMix 9 4), (Format-Float $gmFloat 8 4), (Format-Float $sysFloat 9 4),
        (Format-Float $ratioMix 0 2), (Format-Float $ratioFloat 0 2))
}

function Add-SystemSection {
    for ($p = 1; $p -le $callProfile.Names.Count; $p++) {
        Add-Line
        Add-Line ('---- GameMath against the system library, {0} profile ----' -f $callProfile.Names[$p - 1])
        Add-Line
        Add-Line 'ms per logic frame, and what GameMath adds over the platform.'
        Add-Line
        Add-Line ('{0,-22}  {1,19}  {2,19}' -f '', 'as the game', 'all float')
        Add-Line ('{0,-22}  {1,8} {2,9}  {3,8} {4,9}' -f 'configuration', 'GameMath', 'system', 'GameMath', 'system')
        Add-Line ('{0,-22}  {1,8} {2,9}  {3,8} {4,9}' -f (Get-Bar 22), (Get-Bar 8), (Get-Bar 9), (Get-Bar 8), (Get-Bar 9))
        foreach ($config in $benches.Configs) { Add-SystemRow $config $p }
    }
}

# ---------- ms per frame, one block per profile ----------

function Add-MillisecondsSection {
    param([int]$ProfileIndex)

    $configs = $benches.Configs
    Add-Line
    Add-Line ('---- ms per logic frame, {0} ----' -f (Get-ProfileTitle $ProfileIndex))
    Add-Line

    $dashes = '{0,-12} {1,-8} {2,11}' -f '------------', '--------', '-----------'
    foreach ($config in $configs) { $dashes += ' {0,16}' -f (Get-Bar 16) }
    $line = '{0,-12} {1,-8} {2,11}' -f 'gm function', 'bench', 'calls/frame'
    foreach ($config in $configs) { $line += ' {0,16}' -f $config }
    Add-Line $line
    Add-Line $dashes

    $sums = New-Object 'double[]' $configs.Count
    foreach ($f in $callProfile.Functions) {
        $rate = Get-Rate $f $ProfileIndex
        $line = '{0,-12} {1,-8} {2}' -f $f, (Get-Bench $f), (Format-Float $rate 11 4)
        for ($c = 0; $c -lt $configs.Count; $c++) {
            $ms = $rate * (Get-Ns $configs[$c] $f) / 1e6
            $sums[$c] += $ms
            $line += ' ' + (Format-Float $ms 16 4)
        }
        Add-Line $line
    }

    Add-Line $dashes
    $total   = '{0,-12} {1,-8} {2,11}' -f 'TOTAL', '', ''
    $percent = '{0,-12} {1,-8} {2,11}' -f '% of 16.67 ms', '', ''
    for ($c = 0; $c -lt $configs.Count; $c++) {
        $total   += ' ' + (Format-Float $sums[$c] 16 4)
        $percent += ' ' + (Format-Float ($sums[$c] / 16.667 * 100) 15 2) + '%'
    }
    Add-Line $total
    Add-Line $percent
}

# ---------- _PC_24 against _PC_53 ----------

function Add-PrecisionProfiles {
    param([string]$Config24, [string]$Config53, [int]$Decide)

    Add-Line 'profile        frames       PC24 ms       PC53 ms      delta ms   delta %'
    Add-Line '----------  ----------  ------------  ------------  ------------  --------'
    for ($p = 1; $p -le $callProfile.Names.Count; $p++) {
        $a = Get-FrameCost $Config24 $p
        $b = Get-FrameCost $Config53 $p
        $marker = ''
        if ($p -eq $Decide) { $marker = '   <--' }
        Add-Line ('{0,-10}  {1}  {2}  {3}  {4}  {5}%{6}' -f $callProfile.Names[$p - 1],
            (Format-Integer $callProfile.Frames[$p - 1] 10), (Format-Float $a 12 4), (Format-Float $b 12 4),
            (Format-Float ($a - $b) 12 4 -Plus), (Format-Float (Get-DeltaPercent $a $b) 7 1 -Plus), $marker)
    }
}

function Add-PrecisionFunctions {
    param([string]$Config24, [string]$Config53, [int]$Decide)

    $dashes = '------------  --------  -------------  ------------  ------------  ------------  --------'
    Add-Line 'gm function   bench       calls/frame       PC24 ms       PC53 ms      delta ms   delta %'
    Add-Line $dashes

    $total24 = 0.0; $total53 = 0.0
    foreach ($f in $callProfile.Functions) {
        if ($callProfile.Calls["$f|$Decide"] -eq 0) { continue }
        $rate = Get-Rate $f $Decide
        $a = $rate * (Get-Ns $Config24 $f) / 1e6
        $b = $rate * (Get-Ns $Config53 $f) / 1e6
        $total24 += $a; $total53 += $b
        Add-Line ('{0,-12}  {1,-8}  {2}  {3}  {4}  {5}  {6}%' -f $f, (Get-Bench $f), (Format-Float $rate 13 4),
            (Format-Float $a 12 4), (Format-Float $b 12 4), (Format-Float ($a - $b) 12 4 -Plus),
            (Format-Float (Get-DeltaPercent $a $b) 7 1 -Plus))
    }

    Add-Line $dashes
    Add-Line ('{0,-12}  {1,-8}  {2,13}  {3}  {4}  {5}  {6}%' -f 'TOTAL', '', '',
        (Format-Float $total24 12 4), (Format-Float $total53 12 4), (Format-Float ($total24 - $total53) 12 4 -Plus),
        (Format-Float (Get-DeltaPercent $total24 $total53) 7 1 -Plus))
}

function Add-PrecisionSection {
    $decide = Get-BusiestProfile
    $pairs  = 0

    Add-Line
    Add-Line '---- _PC_24 against _PC_53 ----'
    Add-Line

    foreach ($config in $benches.Configs) {
        if ($config -cnotmatch 'PC24') { continue }
        $mate = ([regex]'PC24').Replace($config, 'PC53', 1)
        if (-not $benches.Seen.ContainsKey($mate)) { continue }

        $pairs++
        $base = ([regex]'-PC24').Replace($config, '', 1)
        Add-Line "$base, ms per logic frame"
        Add-Line
        Add-PrecisionProfiles $config $mate $decide
        Add-Line
        Add-Line ('{0}, where the difference sits, {1} profile' -f $base, $callProfile.Names[$decide - 1])
        Add-Line
        Add-PrecisionFunctions $config $mate $decide
        Add-Line
    }

    if ($pairs -gt 0) { return }
    Add-Line 'No PC24 / PC53 pair among the results. The precision control only'
    Add-Line 'exists on 32-bit x86; run the benchmark there to fill this in.'
    Add-Line
}

# ---------- the run ----------

if (-not (Test-Path (Join-Path $BenchDir $profileName))) {
    throw "call profile not found: $profileName"
}

if (-not (Test-Path (Join-Path $BenchDir $summaryName))) {
    throw 'dump comparison not found: Tests/GameMath/Snapshots/math-summary.txt'
}

$callProfile = Read-CallProfile
$mismatch    = Read-Mismatch
$benches     = Read-Benchmarks
$out         = New-Object System.Text.StringBuilder

Add-Header
Add-CallsSection
Add-NanosecondsSection
Add-MatrixSection
Add-SystemSection
for ($p = 1; $p -le $callProfile.Names.Count; $p++) { Add-MillisecondsSection $p }
Add-PrecisionSection

$text     = $out.ToString()
$encoding = New-Object System.Text.UTF8Encoding $false
[IO.File]::WriteAllText((Join-Path $BenchDir $outName), $text, $encoding)

Write-Host "wrote $outName"
