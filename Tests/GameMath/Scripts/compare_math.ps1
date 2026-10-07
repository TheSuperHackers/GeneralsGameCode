<#
.SYNOPSIS
    Compares every math-*.txt dump against the macOS one and writes
    math-diff.txt and math-summary.txt.

.DESCRIPTION
    The Windows twin of compare_math.sh. It writes the same two files:

        math-diff.txt      the differing lines of each comparison
        math-summary.txt   one table, row kinds down the side, modes across the top

    The macOS dump is the baseline and comes from the repository, so pull first
    if it was refreshed on the Mac.

    Windows writes CRLF, so line endings are normalised before comparing. The
    files are written with LF and no byte order mark, so both scripts produce the
    same output from the same dumps.

    Dumps are compared row by row. They all come from verify_game_math.c, so
    their rows line up; dumps with a different number of rows stop the script.

.PARAMETER MathDir
    Directory holding the dumps, also where the two files are written.
    Default: Tests\GameMath\Snapshots

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Tests\GameMath\Scripts\compare_math.ps1
#>
[CmdletBinding()]
param(
    [string]$MathDir
)

$ErrorActionPreference = 'Stop'

$testsDir = Split-Path $PSScriptRoot -Parent

if (-not $MathDir) { $MathDir = Join-Path $testsDir 'Snapshots' }

$MathDir = (Resolve-Path $MathDir).Path

$diffName    = 'math-diff.txt'
$summaryName = 'math-summary.txt'
$groupOrder  = @('through double', 'float -> double', 'double -> float', 'plain float')

# ---------- dumps ----------

function Read-Dump {
    param([string]$Name)

    $text  = [IO.File]::ReadAllText((Join-Path $MathDir $Name))
    $lines = New-Object System.Collections.Generic.List[string]
    foreach ($line in $text.Replace("`r", '').Split("`n")) {
        if ($line.StartsWith('================')) { continue }
        $lines.Add($line)
    }
    if ($lines.Count -gt 0 -and $lines[$lines.Count - 1] -eq '') {
        $lines.RemoveAt($lines.Count - 1)
    }
    return ,$lines.ToArray()
}

function Get-Label {
    param([string]$Name)

    return ($Name -replace '^math-', '') -replace '\.txt$', ''
}

function Get-ShortLabel {
    param([string]$Label)

    $withoutPlatform = $Label -replace '^win-', ''
    return ([regex]'precise').Replace($withoutPlatform, 'prec', 1)
}

# ---------- row kinds ----------

function Get-RowName {
    param([string]$Line)

    return ($Line.TrimStart() -split '\s+', 2)[0]
}

function Get-Suffix {
    param([string]$RowName)

    $dot = $RowName.IndexOf('.')
    if ($dot -lt 0) { return '' }
    return $RowName.Substring($dot + 1)
}

function Get-Group {
    param([string]$Suffix)

    if ($Suffix -ceq 'f2d')                   { return 'float -> double' }
    if ($Suffix -cmatch '2f$')                { return 'double -> float' }
    if ($Suffix -ceq 'f' -or $Suffix -cmatch '\.f$') { return 'plain float' }
    return 'through double'
}

function Measure-Suffixes {
    param([string[]]$Lines)

    $counts = @{}
    foreach ($line in $Lines) {
        $suffix = Get-Suffix (Get-RowName $line)
        if ($suffix -eq '') { continue }
        $counts[$suffix] = 1 + [int]$counts[$suffix]
    }
    return $counts
}

function Measure-Groups {
    param([hashtable]$Total, [hashtable]$Differing)

    $groups = @{}
    foreach ($suffix in $Total.Keys) {
        $group = Get-Group $suffix
        if (-not $groups.ContainsKey($group)) { $groups[$group] = @{ Dif = 0; Tot = 0 } }
        $groups[$group].Dif += [int]$Differing[$suffix]
        $groups[$group].Tot += $Total[$suffix]
    }
    return $groups
}

# ---------- output ----------

function Add-Line {
    param([System.Text.StringBuilder]$Out, [string]$Text = '')

    [void]$Out.Append($Text).Append("`n")
}

function Write-TextFile {
    param([string]$Name, [string]$Text)

    $encoding = New-Object System.Text.UTF8Encoding $false
    [IO.File]::WriteAllText((Join-Path $MathDir $Name), $Text, $encoding)
}

# ---------- one comparison ----------

function Add-GroupTable {
    param([System.Text.StringBuilder]$Out, [hashtable]$Groups)

    Add-Line $Out ('{0,-18} {1,9} {2,9}' -f 'group', 'differing', 'of total')
    Add-Line $Out ('{0,-18} {1,9} {2,9}' -f '------------------', '---------', '--------')
    foreach ($group in $groupOrder) {
        if (-not $Groups.ContainsKey($group)) { continue }
        Add-Line $Out ('{0,-18} {1,9} {2,9}' -f $group, $Groups[$group].Dif, $Groups[$group].Tot)
    }
}

function Add-RowTable {
    param([System.Text.StringBuilder]$Out, [hashtable]$Total, [hashtable]$Differing)

    $rows = New-Object System.Collections.Generic.List[object]
    foreach ($suffix in $Differing.Keys) {
        $rows.Add([pscustomobject]@{ Name = ".$suffix"; Dif = $Differing[$suffix]; Tot = $Total[$suffix] })
    }
    $rows.Sort([Comparison[object]] {
        param($first, $second)
        if ($first.Dif -ne $second.Dif) { return $second.Dif.CompareTo($first.Dif) }
        return [string]::CompareOrdinal($first.Name, $second.Name)
    })

    Add-Line $Out ('{0,-18} {1,9} {2,9}' -f 'row', 'differing', 'of total')
    Add-Line $Out ('{0,-18} {1,9} {2,9}' -f '------------------', '---------', '--------')
    foreach ($row in $rows) {
        Add-Line $Out ('{0,-18} {1,9} {2,9}' -f $row.Name, $row.Dif, $row.Tot)
    }
}

function Add-DetailTable {
    param([System.Text.StringBuilder]$Out, [string[]]$BaseLines, [string[]]$OtherLines, [string]$Label)

    $line = '{0,-20} {1,-46} {2,-18} {3}'
    Add-Line $Out ($line -f 'row', 'arguments', 'macOS', $Label)
    Add-Line $Out ($line -f ('-' * 20), ('-' * 46), ('-' * 18), ('-' * 18))

    for ($i = 0; $i -lt $BaseLines.Count; $i++) {
        $left  = $BaseLines[$i]
        $right = $OtherLines[$i]
        if ($left -ceq $right)        { continue }
        if ($left.StartsWith('----')) { continue }

        $row       = $left -replace ' .*', ''
        $arguments = ($left -replace '^[^ ]+ +', '') -replace ' +[^ ]+$', ''
        $leftBits  = $left -replace '.* ', ''
        $rightBits = $right -replace '.* ', ''
        Add-Line $Out ($line -f $row, $arguments, $leftBits, $rightBits)
    }
}

function Add-SummaryRecords {
    param($Records, [string]$Config, [hashtable]$Total, [hashtable]$Differing, [hashtable]$Groups)

    $allDif = 0
    $allTot = 0
    foreach ($suffix in $Total.Keys) {
        $Records.Add([pscustomobject]@{ Cfg = $Config; Key = ".$suffix"; Dif = [int]$Differing[$suffix]; Tot = $Total[$suffix] })
        $allDif += [int]$Differing[$suffix]
        $allTot += $Total[$suffix]
    }
    foreach ($group in $Groups.Keys) {
        $Records.Add([pscustomobject]@{ Cfg = $Config; Key = $group; Dif = $Groups[$group].Dif; Tot = $Groups[$group].Tot })
    }
    $Records.Add([pscustomobject]@{ Cfg = $Config; Key = 'total'; Dif = $allDif; Tot = $allTot })
}

function Add-Comparison {
    param([System.Text.StringBuilder]$Out, $Records, [string[]]$BaseLines, [string]$OtherName)

    $otherLines = Read-Dump $OtherName
    if ($otherLines.Count -ne $BaseLines.Count) {
        throw "$OtherName has $($otherLines.Count) rows, $baseName has $($BaseLines.Count); rebuild both dumps from the same verify_game_math.c."
    }

    $label     = Get-Label $OtherName
    $differing = @(for ($i = 0; $i -lt $BaseLines.Count; $i++) { if ($BaseLines[$i] -cne $otherLines[$i]) { $BaseLines[$i] } })

    Add-Line $Out ('================ {0} vs {1} ================' -f (Get-Label $baseName), $label)
    Add-Line $Out ('differing lines: {0}' -f $differing.Count)
    Add-Line $Out

    $total          = Measure-Suffixes $BaseLines
    $differingCount = Measure-Suffixes $differing
    $groups         = Measure-Groups $total $differingCount
    Add-SummaryRecords $Records (Get-ShortLabel $label) $total $differingCount $groups

    if ($differing.Count -gt 0) {
        Add-GroupTable $Out $groups
        Add-Line $Out
        Add-RowTable $Out $total $differingCount
        Add-Line $Out
        Add-DetailTable $Out $BaseLines $otherLines $label
    }
    Add-Line $Out
}

# ---------- the legend ----------

function Add-Legend {
    param([System.Text.StringBuilder]$Out)

    $legend = @(
        'GameMath cross-platform comparison'
        ('generated {0} UTC' -f (Get-Date).ToUniversalTime().ToString('yyyy-MM-dd HH:mm'))
        "baseline $baseName"
        ''
        'Each function is called on the same value several ways. The suffix on'
        'the row name says which way, so a difference can be traced to the'
        'function itself or to a conversion around it.'
        ''
        '  .d     double function, double result        gm_f(x)'
        '  .f     float function, float result          gm_ff((float)x)'
        '  .f2d   float function, result widened        (double)gm_ff((float)x)'
        '  .d2f   double function, result narrowed      (float)gm_f(x)'
        ''
        'Two argument functions vary each argument on its own, since a value can'
        'arrive as a full double or as one that already went through a float,'
        'and each combination is recorded both ways round:'
        ''
        '  .dd     gm_f(x, y)'
        '  .dd2f   (float)gm_f(x, y)'
        '  .df     gm_f(x, (double)(float)y)'
        '  .df2f   (float)gm_f(x, (double)(float)y)'
        '  .fd     gm_f((double)(float)x, y)'
        '  .fd2f   (float)gm_f((double)(float)x, y)'
        '  .ff     gm_f((double)(float)x, (double)(float)y)'
        '  .ff2f   (float)gm_f((double)(float)x, (double)(float)y)'
        ''
        'The .f2d row is what the WWMath wrappers do, so it is the one that'
        'matters for the game.'
        ''
        'Every function is also fed into a small expression, since a result that'
        'is merely stored may be rounded correctly while the same result kept in'
        'a register and used in arithmetic is not. r1, r2 and r3 are the function'
        'applied to three consecutive inputs, in float and in double:'
        ''
        '  .mul.d  .mul.f   r1 * r2'
        '  .inv.d  .inv.f   1 / r1'
        '  .mad.d  .mad.f   r1 * r2 + r3'
        ''
    )
    foreach ($line in $legend) { Add-Line $Out $line }
}

# ---------- the summary ----------

function Get-ColumnKey {
    param([string]$Config)

    $precision = '0'
    if ($Config -cmatch 'PC24')     { $precision = '1' }
    elseif ($Config -cmatch 'PC53') { $precision = '2' }
    return $Config.Substring(0, [Math]::Min(3, $Config.Length)) + $precision + $Config
}

function Format-Summary {
    param($Records)

    $configs = New-Object System.Collections.Generic.List[string]
    $differing = @{}
    $total = @{}
    foreach ($record in $Records) {
        if (-not $configs.Contains($record.Cfg)) { $configs.Add($record.Cfg) }
        $differing["$($record.Key)|$($record.Cfg)"] = $record.Dif
        $total[$record.Key] = $record.Tot
    }
    $configs.Sort([Comparison[string]] { param($first, $second) [string]::CompareOrdinal((Get-ColumnKey $first), (Get-ColumnKey $second)) })

    $rowKeys = [string[]]@($total.Keys | Where-Object { $_.StartsWith('.') })
    [Array]::Sort($rowKeys, [StringComparer]::Ordinal)

    $out = New-Object System.Text.StringBuilder
    Add-Line $out ('Differing lines against {0}, by row kind.' -f (Get-Label $baseName))
    Add-Line $out
    Add-SummaryRow $out 'row kind' 'rows' $configs
    Add-SummaryRow $out ('-' * 18) ('-' * 6) @($configs | ForEach-Object { '-' * 14 })

    foreach ($key in ($groupOrder + 'total')) {
        if (-not $total.ContainsKey($key)) { continue }
        Add-SummaryRow $out $key $total[$key] @($configs | ForEach-Object { [int]$differing["$key|$_"] })
        if ($key -eq 'plain float') { Add-Line $out }
    }
    Add-Line $out
    foreach ($key in $rowKeys) {
        Add-SummaryRow $out $key $total[$key] @($configs | ForEach-Object { [int]$differing["$key|$_"] })
    }
    return $out.ToString()
}

function Add-SummaryRow {
    param([System.Text.StringBuilder]$Out, [string]$Name, $Rows, $Cells)

    $line = '{0,-18} {1,6}' -f $Name, $Rows
    foreach ($cell in $Cells) { $line += ' {0,14}' -f $cell }
    Add-Line $Out $line
}

# ---------- the run ----------

$dumpNames = [string[]]@(Get-ChildItem -Path $MathDir -Filter 'math-*.txt' | ForEach-Object { $_.Name })
[Array]::Sort($dumpNames, [StringComparer]::Ordinal)

$baseName = $dumpNames | Where-Object { $_.StartsWith('math-mac-') } | Select-Object -First 1
if (-not $baseName) {
    throw 'no macOS dump found (math-mac-*.txt)'
}

$otherNames = @($dumpNames | Where-Object { $_ -cne $baseName -and $_ -cne $diffName -and $_ -cne $summaryName })
if ($otherNames.Count -eq 0) {
    throw "nothing to compare against $baseName"
}

$baseLines = Read-Dump $baseName
$records   = New-Object System.Collections.Generic.List[object]
$diffText  = New-Object System.Text.StringBuilder

Add-Legend $diffText
foreach ($otherName in $otherNames) {
    Add-Comparison $diffText $records $baseLines $otherName
}

Write-TextFile $diffName $diffText.ToString()
Write-TextFile $summaryName (Format-Summary $records)

Write-Host "wrote $diffName and $summaryName"
