# Testing

## Test Replays

The GeneralsReplays folder contains replays and the required maps that are tested in CI to ensure that the game is retail compatible.

You can also test with these replays locally:
- Copy the replays into a subfolder in your `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays` folder.
- Copy the maps into `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Maps`
- Start the test with this: (copy into a .bat file next to your executable)
```
START /B /W generalszh.exe -jobs 4 -headless -replay subfolder/*.rep > replay_check.log
echo %errorlevel%
PAUSE
```
It will run the game in the background and check that each replay is compatible. You need to use a VC6 build with optimizations and RTS_BUILD_OPTION_DEBUG = OFF, otherwise the game won't be compatible.

## Code Quality Analysis with clang-tidy

The project includes clang-tidy configuration for static code analysis to help maintain code quality and catch potential bugs.

### Prerequisites

1. **CMake with compile commands export**: The CMake presets already have `CMAKE_EXPORT_COMPILE_COMMANDS=ON` configured.
2. **clang-tidy**: Install clang-tidy for your platform:
   - **Linux**: `apt install clang-tidy` or `yum install clang-tools-extra`
   - **Windows**: Install LLVM or use the version that comes with Visual Studio
   - **macOS**: `brew install llvm` or use Xcode command line tools

### Running clang-tidy

#### Method 1: Using the helper script (Recommended)

The project includes a Python script that simplifies running clang-tidy:

```bash
# Analyze all source files
python3 scripts/run-clang-tidy.py

# Analyze only Core directory
python3 scripts/run-clang-tidy.py --include Core/

# Analyze GeneralsMD but exclude certain patterns
python3 scripts/run-clang-tidy.py --include GeneralsMD/ --exclude test

# Use specific build directory
python3 scripts/run-clang-tidy.py --build-dir build/win32

# Apply fixes automatically (use with caution!)
python3 scripts/run-clang-tidy.py --fix --include Core/Libraries/
```

#### Method 2: Direct clang-tidy usage

First, ensure you have a `compile_commands.json` file:

```bash
# Configure with any preset that exports compile commands
cmake --preset win32  # or vc6, unix, etc.

# Run clang-tidy on specific files
clang-tidy -p build/win32 Core/Libraries/Source/RTS/File.cpp

# Run on multiple files with pattern (Unix/Linux/macOS)
find Core/ -name "*.cpp" | xargs clang-tidy -p build/win32

# Windows Command Prompt alternative
for /r Core\ %i in (*.cpp) do clang-tidy -p build/win32 "%i"

# Windows PowerShell alternative
Get-ChildItem -Path Core\ -Recurse -Filter "*.cpp" | ForEach-Object { clang-tidy -p build/win32 $_.FullName }
```

### Configuration

The `.clang-tidy` file in the project root contains configuration tailored for this legacy C++ codebase:

- **Enabled checks**: Focus on bug-prone patterns, performance issues, and readability
- **Disabled checks**: Overly strict modernization rules that don't fit the legacy codebase
- **Naming conventions**: Adapted to match the existing code style (CamelCase for classes, m_ prefix for members)
- **Header filtering**: Only analyzes project headers, not system/external headers

### Integration with Development Workflow

#### For Contributors

Run clang-tidy on your changes before submitting PRs:

```bash
# Analyze only files you've modified
python3 scripts/run-clang-tidy.py --include "path/to/your/changes/"
```

#### For Maintainers

Consider running periodic full codebase analysis:

```bash
# Full analysis (may take a while)
python3 scripts/run-clang-tidy.py > clang-tidy-report.txt 2>&1
```

### MinGW-w64 Compatibility

The clang-tidy configuration is designed to work with the MinGW-w64 cross-compilation setup. The project supports:

- **MinGW-w64 headers**: For Windows API compatibility
- **ReactOS ATL**: For COM interface support (as referenced in PR #672)
- **Legacy C++98 patterns**: While encouraging modern practices where appropriate

### Troubleshooting

**Issue**: `compile_commands.json not found`
**Solution**: Run cmake configuration first: `cmake --preset <preset-name>`

**Issue**: clang-tidy reports errors in system headers
**Solution**: The configuration should filter these out, but you can also use `--system-headers=false`

**Issue**: Too many warnings for legacy code
**Solution**: Use the `--include` flag to focus on specific directories or files you're working on