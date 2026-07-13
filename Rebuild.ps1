$ErrorActionPreference = "Stop"

Write-Host "Initializing MSVC environment and building project..." -ForegroundColor Cyan

# The command to initialize MSVC environment and then run CMake build & install
$cmd = "call `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat`" x86 && cmake --build build/win32 --config Release --target install"
cmd.exe /c $cmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build and install completed successfully!" -ForegroundColor Green
} else {
    Write-Host "Build failed with exit code $LASTEXITCODE." -ForegroundColor Red
}
