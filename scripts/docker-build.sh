#!/usr/bin/env bash
#
# Build script for compiling Generals/Zero Hour on Linux using Docker
#
# This script builds Windows executables using a Docker container with Wine and VC6.
# The resulting binaries can be run on Linux using Wine or on Windows natively.
#
# Usage:
#   ./scripts/docker-build.sh                  # Full build (both games)
#   ./scripts/docker-build.sh --game zh        # Build Zero Hour only
#   ./scripts/docker-build.sh --game generals  # Build Generals only
#   ./scripts/docker-build.sh --target generalszh  # Build specific target
#   ./scripts/docker-build.sh --clean          # Clean build directory
#   ./scripts/docker-build.sh --interactive    # Enter container shell
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

print_header() {
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}  Generals Game Code - Linux Builder${NC}"
    echo -e "${BLUE}======================================${NC}"
    echo ""
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [TARGET...]

Build Generals/Zero Hour on Linux using Docker (produces Windows executables).

Options:
    -h, --help          Show this help message
    -g, --game GAME     Build specific game: 'zh' (Zero Hour), 'generals', or 'all' (default: all)
    -t, --target TARGET Build specific CMake target (e.g., generalszh, WorldBuilderZH)
    -c, --clean         Clean build directory before building
    -i, --interactive   Enter container shell for debugging
    --cmake             Force CMake reconfiguration
    -j, --jobs N        Number of parallel jobs (passed to ninja)

Examples:
    $(basename "$0")                      # Build everything
    $(basename "$0") --game zh            # Build Zero Hour only
    $(basename "$0") --target generalszh  # Build Zero Hour executable only
    $(basename "$0") --clean --game all   # Clean and rebuild everything
    $(basename "$0") --interactive        # Enter container for debugging

Output:
    Build artifacts are placed in: $BUILD_DIR/

    Zero Hour executables:
        - generalszh.exe       (Main game)
        - WorldBuilderZH.exe   (Map editor)
        - W3DViewZH.exe        (3D model viewer)

    Generals executables:
        - generalsv.exe        (Main game)
        - WorldBuilderV.exe    (Map editor)
        - W3DViewV.exe         (3D model viewer)

Running the game:
    After building, you can run the game with Wine:
        wine $BUILD_DIR/GeneralsMD/generalszh.exe

    Note: You need the original game data files installed.
EOF
}

check_dependencies() {
    print_info "Checking dependencies..."

    if ! command -v docker &>/dev/null; then
        print_error "Docker is not installed. Please install Docker first."
        echo "  See: https://docs.docker.com/engine/install/"
        exit 1
    fi

    if ! docker info &>/dev/null; then
        print_error "Docker daemon is not running or you don't have permission."
        echo "  Try: sudo systemctl start docker"
        echo "  Or add your user to the docker group: sudo usermod -aG docker \$USER"
        exit 1
    fi

    print_success "All dependencies satisfied"
}

build_docker_image() {
    print_info "Building Docker image (this may take a while on first run)..."

    docker build \
        --build-arg UID="$(id -u)" \
        --build-arg GID="$(id -g)" \
        "$PROJECT_DIR/resources/dockerbuild" \
        -t zerohour-build \
        || {
            print_error "Failed to build Docker image"
            exit 1
        }

    print_success "Docker image ready"
}

run_build() {
    local force_cmake="$1"
    local target="$2"
    local interactive="$3"

    local docker_flags=""
    if [[ "$interactive" == "true" ]]; then
        docker_flags="-it --entrypoint bash"
    fi

    print_info "Starting build..."
    if [[ -n "$target" ]]; then
        print_info "Target: $target"
    fi

    # shellcheck disable=SC2086
    docker run \
        -u "$(id -u):$(id -g)" \
        -e MAKE_TARGET="$target" \
        -e FORCE_CMAKE="$force_cmake" \
        -v "$PROJECT_DIR:/build/cnc" \
        --rm \
        $docker_flags \
        zerohour-build \
        || {
            print_error "Build failed"
            exit 1
        }

    if [[ "$interactive" != "true" ]]; then
        print_success "Build completed"
    fi
}

clean_build() {
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    print_success "Build directory cleaned"
}

list_outputs() {
    echo ""
    print_info "Build outputs:"
    echo ""

    if [[ -d "$BUILD_DIR/GeneralsMD" ]]; then
        echo "Zero Hour (GeneralsMD):"
        find "$BUILD_DIR/GeneralsMD" -maxdepth 1 -name "*.exe" -printf "    %f (%s bytes)\n" 2>/dev/null || true
    fi

    if [[ -d "$BUILD_DIR/Generals" ]]; then
        echo ""
        echo "Generals:"
        find "$BUILD_DIR/Generals" -maxdepth 1 -name "*.exe" -printf "    %f (%s bytes)\n" 2>/dev/null || true
    fi
}

# Parse arguments
GAME="all"
TARGET=""
CLEAN=false
INTERACTIVE=false
FORCE_CMAKE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        -g|--game)
            GAME="$2"
            shift 2
            ;;
        -t|--target)
            TARGET="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -i|--interactive)
            INTERACTIVE=true
            shift
            ;;
        --cmake)
            FORCE_CMAKE=true
            shift
            ;;
        -j|--jobs)
            # Jobs are handled by ninja in the container
            shift 2
            ;;
        -*)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
        *)
            # Positional argument treated as target
            TARGET="$TARGET $1"
            shift
            ;;
    esac
done

# Set target based on game selection
if [[ -z "$TARGET" ]]; then
    case "$GAME" in
        zh|zerohour)
            TARGET="generalszh generalszh_tools"
            ;;
        generals)
            TARGET="generalsv generalsv_tools"
            ;;
        all)
            TARGET=""  # Build all
            ;;
        *)
            print_error "Unknown game: $GAME (use 'zh', 'generals', or 'all')"
            exit 1
            ;;
    esac
fi

# Main execution
print_header
check_dependencies

if [[ "$CLEAN" == "true" ]]; then
    clean_build
fi

build_docker_image
run_build "$FORCE_CMAKE" "$TARGET" "$INTERACTIVE"

if [[ "$INTERACTIVE" != "true" ]]; then
    list_outputs

    echo ""
    print_info "To run the game with Wine:"
    echo "    wine $BUILD_DIR/GeneralsMD/generalszh.exe"
fi
