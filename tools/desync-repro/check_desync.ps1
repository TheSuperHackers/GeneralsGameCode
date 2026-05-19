# =============================================================================
#  check_desync.ps1
# =============================================================================
#  Reads the desync flag out of a Generals/Zero Hour .rep file. Useful for
#  scripting the before/after comparison: run the test, scan the userdata
#  Replays directory, and report which replays show desync=1.
#
#  Replay header byte layout (the bytes we care about):
#    0x00 - 0x05  GENREP magic
#    0x06 - 0x09  startTime  (uint32 LE)
#    0x0A - 0x0D  endTime    (uint32 LE)
#    0x0E - 0x11  frameCount (uint32 LE)
#    0x12         desyncGame (1 byte bool)         <-- THIS
#    0x13         quitEarly  (1 byte bool)
#    0x14 - 0x1B  playerDiscons[8] (8 bytes)
#
#  Usage:
#    powershell -ExecutionPolicy Bypass -File check_desync.ps1
#    powershell -ExecutionPolicy Bypass -File check_desync.ps1 -Path 'C:\Users\You\Documents\Command and Conquer Generals Zero Hour Data\Replays'
# =============================================================================

param(
    [string]$Path = (Join-Path $env:USERPROFILE 'Documents\Command and Conquer Generals Zero Hour Data\Replays')
)

if (-not (Test-Path -LiteralPath $Path)) {
    Write-Host "Replay directory not found: $Path" -ForegroundColor Red
    exit 1
}

$results = @()

Get-ChildItem -LiteralPath $Path -Filter *.rep -File | Sort-Object LastWriteTime -Descending | ForEach-Object {
    $rep = $_
    try {
        $fs = [System.IO.File]::OpenRead($rep.FullName)
        $br = New-Object System.IO.BinaryReader($fs)

        $magic = [System.Text.Encoding]::ASCII.GetString($br.ReadBytes(6))
        if ($magic -ne 'GENREP') {
            Write-Host "  [skip] $($rep.Name): not GENREP" -ForegroundColor DarkGray
            return
        }
        $startTime  = $br.ReadUInt32()
        $endTime    = $br.ReadUInt32()
        $frameCount = $br.ReadUInt32()
        $desync     = $br.ReadByte()
        $quitEarly  = $br.ReadByte()

        $color = if ($desync -ne 0) { 'Red' } else { 'Green' }
        $tag   = if ($desync -ne 0) { 'DESYNC' } else { '  ok  ' }
        Write-Host ("  [{0}] frames={1,6}  desync={2}  quit={3}  {4}" -f $tag, $frameCount, $desync, $quitEarly, $rep.Name) -ForegroundColor $color

        $results += [PSCustomObject]@{
            Name       = $rep.Name
            Frames     = $frameCount
            Desync     = $desync
            QuitEarly  = $quitEarly
            FullPath   = $rep.FullName
        }
    } catch {
        Write-Host "  [err]  $($rep.Name): $_" -ForegroundColor Yellow
    } finally {
        if ($br) { $br.Dispose() }
        if ($fs) { $fs.Dispose() }
    }
}

$desyncCount = ($results | Where-Object { $_.Desync -ne 0 }).Count
$totalCount  = $results.Count
Write-Host ''
Write-Host ("Summary: {0} desync / {1} replays scanned in {2}" -f $desyncCount, $totalCount, $Path)
