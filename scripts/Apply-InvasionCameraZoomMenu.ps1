param(
    [Parameter(Mandatory = $true)]
    [string] $ModDirectory,
    [string] $BackupDirectory
)

$ErrorActionPreference = 'Stop'
$mod = (Resolve-Path -LiteralPath $ModDirectory).Path
if (!$BackupDirectory) {
    $BackupDirectory = Join-Path (Split-Path -Parent $mod) ('Backups\CameraZoomMenu-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fffZ'))
}
$backup = [IO.Path]::GetFullPath($BackupDirectory).TrimEnd('\','/')
if ($backup -eq $mod -or $backup.StartsWith($mod + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Choose a backup directory outside the mod directory.'
}

# Preserve the original bytes of the WND wherever text is not changed.
$layoutEncoding = [Text.Encoding]::GetEncoding(28591)
$layoutPath = 'Window\Menus\OptionsMenu.wnd'
$layout = $layoutEncoding.GetString([IO.File]::ReadAllBytes((Join-Path $mod $layoutPath)))
$labelFound = $layout.Contains('NAME = "OptionsMenu.wnd:CameraZoomSpeedLabel";')
$sliderFound = $layout.Contains('NAME = "OptionsMenu.wnd:SliderCameraZoomSpeed";')
if ($labelFound -ne $sliderFound) {
    throw 'OptionsMenu.wnd contains only one of the camera zoom controls.'
}

$updates = [ordered]@{}
if (!$sliderFound) {
    $blocks = [regex]::Matches($layout, '(?ms)^ {8}CHILD\r?\n {8}WINDOW\r?\n.*?^ {8}END\r?\n')
    $scrollLabel = @($blocks | Where-Object { $_.Value.Contains('NAME = "OptionsMenu.wnd:ScrollSpeedLabel";') })
    $scrollSlider = @($blocks | Where-Object { $_.Value.Contains('NAME = "OptionsMenu.wnd:SliderScrollSpeed";') })
    if ($scrollLabel.Count -ne 1 -or $scrollSlider.Count -ne 1) {
        throw 'Could not find the scroll speed controls in the Invasion options layout.'
    }

    $label = $scrollLabel[0].Value
    $slider = $scrollSlider[0].Value
    $rect = 'UPPERLEFT: \d+ \d+,\r?\n([ \t]+)BOTTOMRIGHT: \d+ \d+'
    $newLine = if ($layout.Contains([string][char]13 + [char]10)) { [string][char]13 + [char]10 } else { [string][char]10 }

    $leftLabel = $label -replace $rect, ('UPPERLEFT: 69 350,' + $newLine + '$1BOTTOMRIGHT: 279 368')
    $leftSlider = $slider -replace $rect, ('UPPERLEFT: 69 368,' + $newLine + '$1BOTTOMRIGHT: 279 392')
    $zoomLabel = $label.Replace('OptionsMenu.wnd:ScrollSpeedLabel','OptionsMenu.wnd:CameraZoomSpeedLabel')
    $zoomLabel = $zoomLabel.Replace('TEXT = "GUI:ScrollSpeed";','TEXT = "GUI:CameraZoomSpeed";')
    $zoomLabel = $zoomLabel -replace $rect, ('UPPERLEFT: 303 350,' + $newLine + '$1BOTTOMRIGHT: 513 368')
    $zoomSlider = $slider.Replace('OptionsMenu.wnd:SliderScrollSpeed','OptionsMenu.wnd:SliderCameraZoomSpeed')
    $zoomSlider = $zoomSlider -replace '(?m)^ *TOOLTIPTEXT = "ToolTip:ScrollSpeed";\r?\n', ''
    $zoomSlider = $zoomSlider.Replace('MINVALUE: 10,','MINVALUE: 1,').Replace('MAXVALUE: 145;','MAXVALUE: 64;')
    if (!$zoomSlider.Contains('MINVALUE: 1,') -or !$zoomSlider.Contains('MAXVALUE: 64;')) {
        throw 'The Invasion scroll slider bounds did not match.'
    }
    $zoomSlider = $zoomSlider -replace $rect, ('UPPERLEFT: 303 368,' + $newLine + '$1BOTTOMRIGHT: 513 392')

    $layout = $layout.Replace($label, $leftLabel).Replace($slider, $leftSlider + $zoomLabel + $zoomSlider)
    $updates[$layoutPath] = $layoutEncoding.GetBytes($layout)
}

$key = 'GUI:CameraZoomSpeed'
$ascii = [Text.Encoding]::ASCII
$translations = [ordered]@{
    'Data\English\generals.csf' = 'Zoom speed'
    'Data\English\generals_ru.csf' = 'Скорость зума'
}
foreach ($path in $translations.Keys) {
    $original = [IO.File]::ReadAllBytes((Join-Path $mod $path))
    if ($original.Length -lt 24 -or $ascii.GetString($original, 0, 4) -cne ' FSC') {
        throw "Invalid CSF header in $path"
    }
    if ($ascii.GetString($original).IndexOf($key, [StringComparison]::OrdinalIgnoreCase) -ge 0) {
        continue
    }

    # Append a CSF label with one complemented UTF-16LE string.
    foreach ($offset in @(8, 12)) {
        $count = [BitConverter]::ToInt32($original, $offset)
        [BitConverter]::GetBytes($count + 1).CopyTo($original, $offset)
    }
    $labelBytes = $ascii.GetBytes($key)
    $labelText = $translations[$path]
    $stringBytes = [Text.Encoding]::Unicode.GetBytes($labelText)
    for ($i = 0; $i -lt $stringBytes.Length; $i++) {
        $stringBytes[$i] = $stringBytes[$i] -bxor 255
    }
    $stream = New-Object IO.MemoryStream
    $writer = New-Object IO.BinaryWriter $stream
    try {
        $writer.Write($original)
        $writer.Write($ascii.GetBytes(' LBL'))
        $writer.Write([int]1)
        $writer.Write($labelBytes.Length)
        $writer.Write($labelBytes)
        $writer.Write($ascii.GetBytes(' RTS'))
        $writer.Write($labelText.Length)
        $writer.Write($stringBytes)
        $updates[$path] = $stream.ToArray()
    }
    finally {
        $writer.Dispose()
    }
}

foreach ($path in $updates.Keys) {
    $source = Join-Path $mod $path
    $saved = Join-Path (Join-Path $backup (Split-Path -Leaf $mod)) $path
    if (!(Test-Path -LiteralPath $saved)) {
        New-Item -ItemType Directory -Path (Split-Path -Parent $saved) -Force | Out-Null
        Copy-Item -LiteralPath $source -Destination $saved
    }
    if ((Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $saved).Hash) {
        throw "Backup does not match $source"
    }
}
foreach ($path in $updates.Keys) {
    [IO.File]::WriteAllBytes((Join-Path $mod $path), $updates[$path])
    Write-Output "Updated $path"
}
if ($updates.Count -gt 0) {
    Write-Output "Backups saved in $backup"
}
