[CmdletBinding()]
param(
    [string]$GameDirectory,
    [switch]$Launch
)

$ErrorActionPreference = "Stop"
$installerDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$packageDirectory = Split-Path -Parent (Split-Path -Parent $installerDirectory)
$modDirectory = Join-Path $packageDirectory "TECHBYSAKH_Mod"
$packageExecutable = Join-Path $packageDirectory "generalszh.exe"

function Get-RegistryGamePath {
    $registryLocations = @(
        "HKCU:\SOFTWARE\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour",
        "HKCU:\SOFTWARE\Electronic Arts\EA Games\Generals Zero Hour",
        "HKLM:\SOFTWARE\WOW6432Node\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour",
        "HKLM:\SOFTWARE\WOW6432Node\Electronic Arts\EA Games\Generals Zero Hour",
        "HKLM:\SOFTWARE\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour",
        "HKLM:\SOFTWARE\Electronic Arts\EA Games\Generals Zero Hour"
    )

    foreach ($location in $registryLocations) {
        try {
            $properties = Get-ItemProperty -Path $location -ErrorAction Stop
            if ($properties.InstallPath -and (Test-Path $properties.InstallPath)) {
                return $properties.InstallPath
            }
        } catch {
            # The key is optional; continue searching other known locations.
        }
    }

    return $null
}

if (-not (Test-Path $packageExecutable)) {
    throw "The package is incomplete: generalszh.exe was not found next to the TECHBYSAKH_Mod folder."
}

if ([string]::IsNullOrWhiteSpace($GameDirectory)) {
    $GameDirectory = Get-RegistryGamePath
}

if ([string]::IsNullOrWhiteSpace($GameDirectory)) {
    $GameDirectory = Read-Host "Enter your existing Generals/Zero Hour installation folder"
}

$GameDirectory = [Environment]::ExpandEnvironmentVariables($GameDirectory.Trim('"', ' '))
$stockExecutable = Join-Path $GameDirectory "generalszh.exe"

if (-not (Test-Path $GameDirectory)) {
    throw "The selected game directory does not exist: $GameDirectory"
}

if (-not (Test-Path $stockExecutable)) {
    throw "No existing generalszh.exe was found in $GameDirectory. Install Generals/Zero Hour first, then run this installer again."
}

$backupExecutable = Join-Path $GameDirectory "generalszh.exe.original"
if (-not (Test-Path $backupExecutable)) {
    Copy-Item -LiteralPath $stockExecutable -Destination $backupExecutable
    Write-Host "Backed up the existing executable to $backupExecutable" -ForegroundColor DarkGray
}

Copy-Item -LiteralPath $packageExecutable -Destination $stockExecutable -Force
$installedModDirectory = Join-Path $GameDirectory "TECHBYSAKH_Mod"
if (Test-Path $modDirectory) {
    Copy-Item -LiteralPath $modDirectory -Destination $installedModDirectory -Recurse -Force
}

$launcherPath = Join-Path $GameDirectory "Launch-TECHBYSAKH.cmd"
$launcherText = @"
@echo off
setlocal
cd /d "%~dp0"
start "TECHBYSAKH Modified" "%~dp0generalszh.exe" %*
endlocal
"@
Set-Content -LiteralPath $launcherPath -Value $launcherText -Encoding ASCII

Write-Host "TECHBYSAKH Modified installed successfully." -ForegroundColor Green
Write-Host "Game directory: $GameDirectory"
Write-Host "Launcher: $launcherPath"
Write-Host "The original executable is preserved as: $backupExecutable"

if ($Launch) {
    Start-Process -FilePath $launcherPath -WorkingDirectory $GameDirectory
}
