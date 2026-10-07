<#
.SYNOPSIS
    Turns a raw counter dump into a call profile: calls per logic frame, per
    function.

.DESCRIPTION
    The Windows twin of profile_from_counts.sh. It prints the same output.

        profile_from_counts.ps1 <dump>                    list the sessions
        profile_from_counts.ps1 <dump> <session>          whole session
        profile_from_counts.ps1 <dump> <session> <a>-<b>  blocks a to b

    A fourth argument names the profile column; it defaults to "calls".

    The raw dump is what the in-game counters write: a block every 100 logic
    frames, each block a list of "gm_function count". A block whose frame
    number is not greater than the one before it starts a new session. Sessions
    are numbered from 1 in the order they were played.

    Nothing here knows what a call costs, and that is deliberate. A call count
    is a fact about the game; a nanosecond is a fact about one implementation
    on one machine. Selecting a load window by cost would bake that
    implementation into the profile, and the profile exists precisely to
    compare implementations. Windows are chosen by wall clock or by call
    volume, never by time per call.

    The output is the body of callcounts-zh.txt. Numbers are rounded the way
    the C printf of awk rounds them, so both scripts print the same text.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\profile_from_counts.ps1 counters.txt

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\profile_from_counts.ps1 counters.txt 2 10-40
#>
param(
    [Parameter(Position = 0)][string]$Dump,
    [Parameter(Position = 1)][string]$Session,
    [Parameter(Position = 2)][string]$Blocks,
    [Parameter(Position = 3)][string]$Name = 'calls'
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Numerics

# gm_sqrtf is the float column of the sqrt row, gm_sqrt the double one.
$benchmarkRows = @(
    'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'atan2', 'sinh', 'cosh', 'tanh', 'asinh',
    'acosh', 'atanh', 'exp', 'exp2', 'expm1', 'log', 'log10', 'log1p', 'logb', 'pow',
    'sqrt', 'cbrt', 'hypot', 'ceil', 'floor', 'trunc', 'round', 'rint', 'fabs', 'fmod',
    'remainder', 'copysign', 'fmax', 'fmin', 'erf', 'erfc', 'lgamma', 'j0', 'y0'
)

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
    param([double]$Value, [int]$Width, [int]$Decimals)

    return (Format-Fixed $Value $Decimals).PadLeft($Width)
}

function Format-Integer {
    param([double]$Value, [int]$Width = 0)

    return ([long][Math]::Truncate($Value)).ToString().PadLeft($Width)
}

function ConvertTo-Number {
    param([string]$Text)

    $match = [regex]::Match([string]$Text, '^\s*[+-]?(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?')
    if (-not $match.Success) { return 0.0 }
    return [double]::Parse($match.Value.Trim(), [Globalization.CultureInfo]::InvariantCulture)
}

# ---------- reading the dump ----------

# lrint has no row of its own in the benchmark; rint stands in for it.
function Get-BenchmarkCell {
    param([string]$Function)

    $base = $Function -replace '^gm_', ''
    if ($base -ceq 'lrint')  { return @('rint', 'd') }
    if ($base -ceq 'lrintf') { return @('rint', 'f') }
    if ($base.EndsWith('f')) {
        $stem = $base.Substring(0, $base.Length - 1)
        if ($benchmarkRows -ccontains $stem) { return @($stem, 'f') }
    }
    return @($base, 'd')
}

function Test-InWindow {
    param([int]$CurrentSession, [int]$CurrentBlock)

    if ($Session -eq '') { return $false }
    if ($CurrentSession + 1 -ne (ConvertTo-Number $Session)) { return $false }
    if ($Blocks -eq '') { return $true }

    $range = $Blocks.Split('-')
    $last  = $range[0]
    if ($range.Count -gt 1 -and $range[1] -ne '') { $last = $range[1] }
    return ($CurrentBlock -ge (ConvertTo-Number $range[0]) -and $CurrentBlock -le (ConvertTo-Number $last))
}

function New-Table {
    return New-Object System.Collections.Hashtable ([StringComparer]::Ordinal)
}

function Split-Fields {
    param([string]$Line)

    return ,[string[]]@(-split $Line | Where-Object { $_ -ne '' })
}

function Read-Dump {
    $state = @{
        Session = 0; Block = 0; PreviousFrame = 0.0; InWindow = $false; Frames = 0.0
        SessionBlocks = @{}; SessionFrames = @{}
        Functions = New-Object System.Collections.Generic.List[string]
        Cell = New-Table; Run = New-Table; Sum = New-Table
    }

    $text = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $Dump).Path)
    foreach ($line in $text.Replace("`r", '').Split("`n")) {
        $fields = Split-Fields $line
        if ($line.StartsWith('==== frame')) {
            Add-FrameHeader $state $fields
            continue
        }
        if ($fields.Count -eq 2 -and $fields[1] -match '^[0-9]+$') {
            Add-Count $state $fields[0] ([double]$fields[1])
        }
    }
    return $state
}

function Add-FrameHeader {
    param($State, [string[]]$Fields)

    $frame       = ConvertTo-Number $Fields[2]
    $blockFrames = ConvertTo-Number $Fields[3]
    if ($frame -le $State.PreviousFrame) {
        $State.Session++
        $State.Block = 0
    }
    $State.PreviousFrame = $frame
    $State.Block++
    $State.SessionBlocks[$State.Session] = $State.Block
    $State.SessionFrames[$State.Session] = [double]$State.SessionFrames[$State.Session] + $blockFrames

    $State.InWindow = Test-InWindow $State.Session $State.Block
    if ($State.InWindow) { $State.Frames += $blockFrames }
}

function Add-Count {
    param($State, [string]$Function, [double]$Count)

    if (-not $State.Cell.ContainsKey($Function)) {
        $State.Functions.Add($Function)
        $State.Cell[$Function] = Get-BenchmarkCell $Function
    }
    $State.Run[$Function] = [double]$State.Run[$Function] + $Count
    if ($State.InWindow) { $State.Sum[$Function] = [double]$State.Sum[$Function] + $Count }
}

# ---------- output ----------

function Write-Sessions {
    param($State)

    Write-Output ('{0,-10} {1,10} {2,12} {3,14}' -f 'session', 'blocks', 'frames', 'minutes at 60Hz')
    for ($s = 0; $s -le $State.Session; $s++) {
        $frames = [double]$State.SessionFrames[$s]
        Write-Output ('{0} {1} {2} {3}' -f (Format-Integer ($s + 1)).PadRight(10),
            (Format-Integer ([double]$State.SessionBlocks[$s]) 10), (Format-Integer $frames 12), (Format-Float ($frames / 3600) 14 1))
    }
    Write-Output ''
    Write-Output 'Pick one: powershell -ExecutionPolicy Bypass -File profile_from_counts.ps1 <dump> <session> [first-last]'
}

# Busiest first, by call volume in the window, then over the whole dump so a
# function that is quiet here but busy elsewhere does not sink to the bottom.
# No timing is involved anywhere.
function Get-RankedFunctions {
    param($State)

    $ranked = $State.Functions.ToArray()
    for ($i = 1; $i -lt $ranked.Count; $i++) {
        for ($j = 0; $j -lt $ranked.Count - $i; $j++) {
            $a = $ranked[$j]; $b = $ranked[$j + 1]
            $sumA = [double]$State.Sum[$a]; $sumB = [double]$State.Sum[$b]
            if ($sumA -lt $sumB -or ($sumA -eq $sumB -and [double]$State.Run[$a] -lt [double]$State.Run[$b])) {
                $ranked[$j] = $b; $ranked[$j + 1] = $a
            }
        }
    }
    return ,$ranked
}

function Write-CallProfile {
    param($State)

    $frames = $State.Frames
    $title  = "# session $Session"
    if ($Blocks -ne '') { $title += ", blocks $Blocks" }
    Write-Output ('{0}, {1} frames, {2} minutes at 60 Hz' -f $title, (Format-Integer $frames), (Format-Fixed ($frames / 3600) 1))
    Write-Output '# calls counted in the window, exactly as the counters recorded'
    Write-Output '# them. The trailing rate is for reading only; the coefficient is'
    Write-Output '# worked out from the counts where it is used, so nothing is lost'
    Write-Output '# to rounding on the way.'
    Write-Output ''

    Write-Output ('# {0,-12} {1,-7} {2,-5} {3,14} {4,12}' -f '', '', '', 'calls', 'per frame')
    Write-Output ('{0,-14} {1,-7} {2,-5} {3,14}' -f 'profiles', '', '', $Name)
    Write-Output ('{0,-14} {1,-7} {2,-5} {3}' -f 'frames', '', '', (Format-Integer $frames 14))
    Write-Output ''

    $total = 0.0
    foreach ($f in (Get-RankedFunctions $State)) {
        $sum  = [double]$State.Sum[$f]
        $cell = $State.Cell[$f]
        Write-Output ('{0,-14} {1,-7} {2,-5} {3} {4}' -f $f, $cell[0], $cell[1], (Format-Integer $sum 14), (Format-Float ($sum / $frames) 12 1))
        $total += $sum
    }
    Write-Output ''
    Write-Output ('# total {0} calls, {1} per frame' -f (Format-Integer $total), (Format-Fixed ($total / $frames) 1))
}

# ---------- the run ----------

if (-not $Dump -or -not (Test-Path -LiteralPath $Dump -PathType Leaf)) {
    [Console]::Error.WriteLine('usage: profile_from_counts.ps1 <dump> [session] [first-last] [name]')
    exit 1
}

$state = Read-Dump

if ($Session -eq '') {
    Write-Sessions $state
    exit 0
}

if ($state.Frames -eq 0) {
    [Console]::Error.WriteLine('the selected window holds no blocks')
    exit 1
}

Write-CallProfile $state
