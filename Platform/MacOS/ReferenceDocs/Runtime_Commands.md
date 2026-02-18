# Runtime & Launch Configuration
**Source:** `GeneralsX:old-multiplatform-attempt` (`docs/ETC/MACOS_BUILD_INSTRUCTIONS.md`, `PHASE34/README.md`)
**Time Context:** Late 2025

## Execution Environment
The ported executable (`GeneralsXZH`) requires specific environment variables and command-line flags to operate correctly on macOS without a full `.app` bundle structure.

### Environment Variables
- `USE_METAL=1`: Force the game to use the native Metal backend. This was the most stable configuration.
- `USE_OPENGL=1`: Used during early development but later deprecated due to driver crashes in `AppleMetalOpenGLRenderer`.
- `CCACHE_DIR=$HOME/.ccache`: Used to speed up compilation (highly recommended for ARM64).

### Command Line Switches
- `-quickstart`: Skips splash screens and movie sequences. Essential for rapid debugging.
- `-win`: Forces windowed mode. Native fullscreen was problematic in early phases.
- `-noshellmap`: Disables the animated menu background (useful for stability testing).
- `-jobs N`: Controls threading (e.g., `-jobs 4`).

### Data Paths
The game expects assets in the current working directory or `$HOME/GeneralsX/GeneralsMD/`.
User data (saves, options.ini) is redirected to:
`~/Library/Application Support/Command and Conquer Generals Zero Hour Data/`

## Typical Launch Script (Pattern)
```bash
#!/bin/bash
# Navigate to game assets
cd "$HOME/GeneralsGameCode/GeneralsMD"

# Force Metal backend and launch
USE_METAL=1 ../build/macos/GeneralsExecutable -quickstart -win 2>&1 | tee ../run.log
```
