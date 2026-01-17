#!/usr/bin/env bash
#
# Install built executables to an existing Generals/Zero Hour installation
#
# This script copies the built executables and tools to a game installation,
# allowing you to test your compiled version with the full game data.
#
# Usage:
#   ./scripts/docker-install.sh /path/to/game/installation
#   ./scripts/docker-install.sh --detect        # Auto-detect game location
#   ./scripts/docker-install.sh --restore       # Restore original files
#

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build/docker"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [GAME_DIR]

Install built executables to an existing Generals/Zero Hour installation.

Arguments:
    GAME_DIR        Path to game installation (containing Data/ subdirectory)

Options:
    -h, --help      Show this help message
    -d, --detect    Auto-detect game installation location
    -r, --restore   Restore original game files from backups
    -g, --game      Which game to install: 'zh' (Zero Hour), 'generals', or 'both'
    --dry-run       Show what would be installed without actually copying

Requirements:
    - Build the project first using: ./scripts/docker-build.sh
    - Have an existing Generals/Zero Hour installation with game data files

The script will:
    1. Backup your original executables (*.exe.backup)
    2. Copy the newly built executables
    3. Copy any new DLLs (DebugWindow.dll, ParticleEditor.dll)
    4. Preserve your original mss32.dll and binkw32.dll (audio/video libraries)

Example:
    # Linux with Wine
    ./scripts/docker-install.sh ~/.wine/drive_c/Program\ Files/EA\ Games/Command\ and\ Conquer\ Generals\ Zero\ Hour

    # Windows (from Git Bash or WSL)
    ./scripts/docker-install.sh "C:/Program Files/EA Games/Command and Conquer Generals Zero Hour"

To restore original files:
    ./scripts/docker-install.sh --restore /path/to/game
EOF
}

detect_game() {
    local candidates=()

    # Check Windows registry (for Git Bash/WSL)
    if command -v reg.exe &>/dev/null; then
        for key in \
            "HKLM\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour" \
            "HKLM\\SOFTWARE\\WOW6432Node\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour"; do
            local install_path
            install_path=$(reg.exe query "$key" //v "InstallPath" 2>/dev/null | grep -oP 'REG_SZ\s+\K.*' | tr -d '\r')
            if [ -n "$install_path" ] && [ -d "$install_path/Data" ]; then
                candidates+=("$install_path")
            fi
        done
    fi

    # Check Wine registry (for Linux with Wine)
    for reg_file in ~/.wine/system.reg ~/.wine32/system.reg; do
        if [ -f "$reg_file" ]; then
            local install_path
            install_path=$(grep -A1 '\[Software\\\\Electronic Arts\\\\EA Games\\\\Command and Conquer Generals Zero Hour\]' "$reg_file" 2>/dev/null | grep -oP '"InstallPath"="\K[^"]+' | sed 's/\\\\x0//g; s/\\\\/\//g; s/^C:/~\/.wine\/drive_c/')
            if [ -n "$install_path" ] && [ -d "$install_path/Data" ]; then
                candidates+=("$install_path")
            fi
        fi
    done

    # Fallback: Common Linux Wine locations
    for prefix in ~/.wine ~/.wine32 ~/.wine-*; do
        if [ -d "$prefix" ]; then
            for path in \
                "$prefix/drive_c/Program Files/EA Games/Command and Conquer Generals Zero Hour" \
                "$prefix/drive_c/Program Files (x86)/EA Games/Command and Conquer Generals Zero Hour" \
                "$prefix/drive_c/Program Files/DODI-Repacks/Generals Zero Hour" \
                "$prefix/drive_c/GOG Games/Command Conquer Generals Zero Hour"; do
                if [ -d "$path/Data" ]; then
                    candidates+=("$path")
                fi
            done
        fi
    done

    # Steam Proton prefixes (for games run through Proton)
    for prefix in ~/.local/share/Steam/steamapps/compatdata/*/pfx; do
        if [ -d "$prefix" ]; then
            for path in \
                "$prefix/drive_c/Program Files/EA Games/Command and Conquer Generals Zero Hour" \
                "$prefix/drive_c/Program Files (x86)/EA Games/Command and Conquer Generals Zero Hour"; do
                if [ -d "$path/Data" ]; then
                    candidates+=("$path")
                fi
            done
        fi
    done

    # Steam native game installations (steamapps/common)
    for path in \
        ~/.local/share/Steam/steamapps/common/"Command and Conquer Generals Zero Hour" \
        ~/.local/share/Steam/steamapps/common/"Command & Conquer Generals - Zero Hour" \
        ~/.steam/steam/steamapps/common/"Command and Conquer Generals Zero Hour" \
        ~/.steam/steam/steamapps/common/"Command & Conquer Generals - Zero Hour"; do
        if [ -d "$path/Data" ]; then
            candidates+=("$path")
        fi
    done

    # Fallback: Windows paths (for Git Bash/WSL)
    for path in \
        "/c/Program Files/EA Games/Command and Conquer Generals Zero Hour" \
        "/c/Program Files (x86)/EA Games/Command and Conquer Generals Zero Hour" \
        "C:/Program Files/EA Games/Command and Conquer Generals Zero Hour" \
        "C:/Program Files (x86)/EA Games/Command and Conquer Generals Zero Hour"; do
        if [ -d "$path/Data" ]; then
            candidates+=("$path")
        fi
    done

    if [ ${#candidates[@]} -eq 0 ]; then
        print_error "No game installation found" >&2
        echo "" >&2
        echo "Please specify the game directory manually:" >&2
        echo "  $(basename "$0") /path/to/game/installation" >&2
        exit 1
    fi

    if [ ${#candidates[@]} -eq 1 ]; then
        echo "${candidates[0]}"
        return
    fi

    echo "Found multiple installations:" >&2
    for i in "${!candidates[@]}"; do
        echo "  $((i+1)). ${candidates[$i]}" >&2
    done
    echo "" >&2
    read -p "Select installation (1-${#candidates[@]}): " selection >&2
    if [[ ! "$selection" =~ ^[0-9]+$ ]] || [ "$selection" -lt 1 ] || [ "$selection" -gt "${#candidates[@]}" ]; then
        print_error "Invalid selection" >&2
        exit 1
    fi
    echo "${candidates[$((selection-1))]}"
}

check_build() {
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found: $BUILD_DIR"
        echo ""
        echo "Please build the project first:"
        echo "  ./scripts/docker-build.sh"
        exit 1
    fi

    if [ ! -f "$BUILD_DIR/GeneralsMD/generalszh.exe" ]; then
        print_error "Built executables not found"
        echo ""
        echo "Please build the project first:"
        echo "  ./scripts/docker-build.sh"
        exit 1
    fi

    print_success "Build directory found"
}

backup_file() {
    local file="$1"
    local backup="${file}.backup"

    if [ -f "$file" ] && [ ! -f "$backup" ]; then
        cp "$file" "$backup"
        print_info "Backed up: $(basename "$file")"
    fi
}

install_file() {
    local src="$1"
    local dest="$2"
    local dry_run="${3:-false}"

    if [ "$dry_run" == "true" ]; then
        echo "  Would copy: $(basename "$src") -> $dest"
    else
        backup_file "$dest"
        cp "$src" "$dest"
        print_success "Installed: $(basename "$src")"
    fi
}

install_game() {
    local game_dir="$1"
    local game_type="$2"  # "zh" or "generals"
    local dry_run="${3:-false}"

    local data_dir="$game_dir/Data"
    local build_game_dir
    local exe_name
    local tools_prefix

    if [ "$game_type" == "zh" ]; then
        build_game_dir="$BUILD_DIR/GeneralsMD"
        exe_name="generalszh.exe"
        tools_prefix="ZH"
    else
        build_game_dir="$BUILD_DIR/Generals"
        exe_name="generalsv.exe"
        tools_prefix="V"
    fi

    if [ ! -d "$build_game_dir" ]; then
        print_warning "Build for $game_type not found, skipping"
        return
    fi

    print_info "Installing $game_type executables..."

    # Main game executable
    if [ -f "$build_game_dir/$exe_name" ]; then
        install_file "$build_game_dir/$exe_name" "$data_dir/$exe_name" "$dry_run"
    fi

    # Tools
    for tool in WorldBuilder$tools_prefix W3DView$tools_prefix guiedit imagepacker mapcachebuilder wdump; do
        local tool_exe="${tool}.exe"
        if [ -f "$build_game_dir/$tool_exe" ]; then
            install_file "$build_game_dir/$tool_exe" "$data_dir/$tool_exe" "$dry_run"
        fi
    done

    # DLLs (but NOT mss32.dll or binkw32.dll - keep originals)
    for dll in DebugWindow.dll ParticleEditor.dll; do
        if [ -f "$build_game_dir/$dll" ]; then
            install_file "$build_game_dir/$dll" "$data_dir/$dll" "$dry_run"
        fi
    done

    # Also check Core directory for DLLs
    if [ -f "$BUILD_DIR/Core/DebugWindow.dll" ]; then
        install_file "$BUILD_DIR/Core/DebugWindow.dll" "$data_dir/DebugWindow.dll" "$dry_run"
    fi
}

restore_files() {
    local game_dir="$1"
    local data_dir="$game_dir/Data"

    print_info "Restoring original files..."

    local restored=0
    for backup in "$data_dir"/*.backup; do
        if [ -f "$backup" ]; then
            local original="${backup%.backup}"
            cp "$backup" "$original"
            print_success "Restored: $(basename "$original")"
            ((restored++))
        fi
    done

    if [ $restored -eq 0 ]; then
        print_warning "No backup files found to restore"
    else
        print_success "Restored $restored files"
    fi
}

# Parse arguments
GAME_DIR=""
DETECT=false
RESTORE=false
GAME_TYPE="zh"
DRY_RUN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        -d|--detect)
            DETECT=true
            shift
            ;;
        -r|--restore)
            RESTORE=true
            shift
            ;;
        -g|--game)
            GAME_TYPE="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        -*)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
        *)
            GAME_DIR="$1"
            shift
            ;;
    esac
done

# Main execution
echo ""
echo "==================================="
echo "  Generals Game Code - Installer"
echo "==================================="
echo ""

# Detect or validate game directory
if [ "$DETECT" == "true" ] || [ -z "$GAME_DIR" ]; then
    print_info "Searching for game installations..."
    GAME_DIR=$(detect_game)
fi

print_info "Game directory: $GAME_DIR"

# Validate game directory
if [ ! -d "$GAME_DIR/Data" ]; then
    print_error "Invalid game directory (Data/ subdirectory not found)"
    echo ""
    echo "Expected directory structure:"
    echo "  $GAME_DIR/"
    echo "    Data/"
    echo "      generalszh.exe"
    echo "      *.big files"
    exit 1
fi

# Restore mode
if [ "$RESTORE" == "true" ]; then
    restore_files "$GAME_DIR"
    exit 0
fi

# Check build exists
check_build

# Install
if [ "$DRY_RUN" == "true" ]; then
    print_info "Dry run - showing what would be installed:"
fi

case "$GAME_TYPE" in
    zh|zerohour)
        install_game "$GAME_DIR" "zh" "$DRY_RUN"
        ;;
    generals)
        install_game "$GAME_DIR" "generals" "$DRY_RUN"
        ;;
    both|all)
        install_game "$GAME_DIR" "zh" "$DRY_RUN"
        install_game "$GAME_DIR" "generals" "$DRY_RUN"
        ;;
    *)
        print_error "Unknown game type: $GAME_TYPE"
        exit 1
        ;;
esac

if [ "$DRY_RUN" != "true" ]; then
    echo ""
    print_success "Installation complete!"
    echo ""
    echo "To run the game:"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "  wine \"$GAME_DIR/Data/generalszh.exe\""
    else
        echo "  cd \"$GAME_DIR/Data\" && generalszh.exe"
    fi
    echo ""
    echo "To restore original files:"
    echo "  $(basename "$0") --restore \"$GAME_DIR\""
fi
