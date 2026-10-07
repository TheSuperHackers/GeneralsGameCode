#!/bin/sh
#
# Builds and runs bench_game_math.c against a GameMath of its own and leaves
# only the result file behind.
#
#   sh run_bench_game_math.sh
#   sh run_bench_game_math.sh --intrinsics off
#   sh run_bench_game_math.sh --source ../../build/macos/_deps/gamemath-src
#
# The macOS side of run_bench_game_math.ps1. There is no /fp matrix and no x87
# precision control outside Windows, so one run produces one file:
#
#     macOS ARM64  ->  bench-mac-arm64-clang.txt
#
# Nothing in the repository has to be configured first, and no build tree of the
# game is read: the GameMath sources are cloned into a work tree outside the
# repository, at the revision ../../cmake/gamemath.cmake pins when that file is
# there and at --rev otherwise.
#
# The library is built Release unless told otherwise. This matters more than it
# looks: an unoptimised GameMath reads about three times slower, which is enough
# to make ARM look slower than x86 when it is not.
#
# This is a timing run. Give it a quiet machine.

set -e

if [ "$(uname -s)" != Darwin ]; then
    echo "only macOS builds are supported" >&2
    exit 1
fi

usage() {
    cat <<'EOF'
usage: sh run_bench_game_math.sh [options]

  --source DIR       GameMath sources to build; skips the clone
  --repo URL         repository to clone from     (default: the pin)
  --rev REV          revision to check out        (default: the pin)
  --config CFG       Release or Debug             (default: Release)
  --intrinsics on|off
                     GM_ENABLE_INTRINSICS; left at GameMath's own default
                     when not given
  --gamemath-dir DIR work tree for the sources and the library
  --out DIR          where the result is written  (default: Benchmarks/GameMath/Snapshots)
  --cc COMPILER      C compiler                   (default: $CC, else cc)
  --rebuild          delete the work tree and build GameMath again
  --keep-exe         copy the executable to --out instead of discarding it
  -h, --help         this text
EOF
}

TESTS_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PIN_FILE=$TESTS_DIR/../../cmake/gamemath.cmake
DEFAULT_REPO=https://github.com/OmniBlade/gamemath.git

SOURCE_DIR=
REPO=
REV=
CONFIG=Release
INTRINSICS=
GAMEMATH_DIR=
OUT_DIR=$TESTS_DIR/Snapshots
CC_BIN=${CC:-cc}
REBUILD=0
KEEP_EXE=0

while [ $# -gt 0 ]; do
    case $1 in
        --source|--repo|--rev|--config|--intrinsics|--gamemath-dir|--out|--cc)
            if [ $# -lt 2 ]; then
                echo "$1 needs a value" >&2
                exit 2
            fi
            ;;
    esac

    case $1 in
        --source)        SOURCE_DIR=$2;   shift 2 ;;
        --repo)          REPO=$2;         shift 2 ;;
        --rev)           REV=$2;          shift 2 ;;
        --config)        CONFIG=$2;       shift 2 ;;
        --intrinsics)    INTRINSICS=$2;   shift 2 ;;
        --gamemath-dir)  GAMEMATH_DIR=$2; shift 2 ;;
        --out)           OUT_DIR=$2;      shift 2 ;;
        --cc)            CC_BIN=$2;       shift 2 ;;
        --rebuild)       REBUILD=1;       shift ;;
        --keep-exe)      KEEP_EXE=1;      shift ;;
        -h|--help)       usage; exit 0 ;;
        *)               echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

case $INTRINSICS in
    ''|on|ON|off|OFF) ;;
    *) echo "--intrinsics takes on or off, not $INTRINSICS" >&2; exit 2 ;;
esac

if [ "$CONFIG" != Release ]; then
    echo "warning: timing a $CONFIG library; the numbers are not comparable to the Release ones" >&2
fi

# ---------- what to build ----------

read_pin() {
    if [ ! -f "$PIN_FILE" ]; then
        return
    fi

    [ -n "$REV" ]  || REV=$(awk '$1 == "GIT_TAG" { print $2 }' "$PIN_FILE")
    [ -n "$REPO" ] || REPO=$(awk '$1 == "GIT_REPOSITORY" { print $2 }' "$PIN_FILE")
}

if [ -z "$SOURCE_DIR" ]; then
    read_pin
    [ -n "$REPO" ] || REPO=$DEFAULT_REPO

    if [ -z "$REV" ]; then
        echo "no revision to build: pass --rev, or run from a checkout that has cmake/gamemath.cmake" >&2
        exit 2
    fi
fi

# A tree built with intrinsics off is not the same library as one built with
# them on, so the setting is part of the name and the two never collide.
if [ -z "$GAMEMATH_DIR" ]; then
    GM_TAG=$(printf '%s' "${REV:-local}" | tr -c 'A-Za-z0-9.' '-' | cut -c1-12)
    GAMEMATH_DIR=${TMPDIR:-/tmp}/gamemath-$GM_TAG-$CONFIG${INTRINSICS:+-intrin-$INTRINSICS}
fi

if [ "$REBUILD" -eq 1 ] && [ -d "$GAMEMATH_DIR" ]; then
    rm -rf "$GAMEMATH_DIR"
fi

mkdir -p "$GAMEMATH_DIR" "$OUT_DIR"
GAMEMATH_DIR=$(CDPATH= cd -- "$GAMEMATH_DIR" && pwd)
OUT_DIR=$(CDPATH= cd -- "$OUT_DIR" && pwd)

SRC_DIR=$GAMEMATH_DIR/src
BUILD_DIR=$GAMEMATH_DIR/build
PROGRAM=$TESTS_DIR/Source/bench_game_math.c

if [ ! -f "$PROGRAM" ]; then
    echo "source not found: $PROGRAM" >&2
    exit 1
fi

if [ -n "$SOURCE_DIR" ]; then
    if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
        echo "GameMath sources not found: $SOURCE_DIR" >&2
        exit 1
    fi
    SRC_DIR=$(CDPATH= cd -- "$SOURCE_DIR" && pwd)
fi

WORK=$(mktemp -d "${TMPDIR:-/tmp}/gamemath-bench-XXXXXX")
trap 'rm -rf "$WORK"' EXIT INT TERM

# ---------- the library ----------

checkout_revision() {
    if [ ! -d "$SRC_DIR/.git" ]; then
        echo "cloning $REPO ..."
        git clone --quiet "$REPO" "$SRC_DIR"
    fi

    if ! git -C "$SRC_DIR" rev-parse --verify --quiet "$REV^{commit}" >/dev/null; then
        git -C "$SRC_DIR" fetch --quiet origin "$REV" \
            || git -C "$SRC_DIR" fetch --quiet origin
    fi

    if git -C "$SRC_DIR" rev-parse --verify --quiet "$REV^{commit}" >/dev/null; then
        git -C "$SRC_DIR" checkout --quiet --detach "$REV"
        return
    fi

    git -C "$SRC_DIR" checkout --quiet --detach "origin/$REV"
}

run_logged() {
    LOG=$WORK/$1
    shift

    if "$@" >"$LOG" 2>&1; then
        return
    fi

    cat "$LOG" >&2
    echo "failed: $*" >&2
    exit 1
}

build_gamemath() {
    CMAKE_OPTS="-DCMAKE_BUILD_TYPE=$CONFIG -DGM_ENABLE_TESTS=OFF"
    if [ -n "$INTRINSICS" ]; then
        CMAKE_OPTS="$CMAKE_OPTS -DGM_ENABLE_INTRINSICS=$INTRINSICS"
    fi

    run_logged cmake-configure.log cmake -S "$SRC_DIR" -B "$BUILD_DIR" $CMAKE_OPTS
    run_logged cmake-build.log cmake --build "$BUILD_DIR" -j
}

find_library() {
    find "$BUILD_DIR" -type f -name 'libgm.a' | head -1
}

if [ -z "$SOURCE_DIR" ]; then
    checkout_revision
fi

GM_REV=unknown
if [ -d "$SRC_DIR/.git" ]; then
    GM_REV=$(git -C "$SRC_DIR" rev-parse --short HEAD)
fi

echo "sources   : $SRC_DIR"
if [ -d "$SRC_DIR/.git" ]; then
    echo "revision  : $(git -C "$SRC_DIR" log --oneline -1)"
fi
echo "config    : $CONFIG, intrinsics ${INTRINSICS:-default}"
echo "output    : $OUT_DIR"

build_gamemath

GM_LIB=$(find_library)
if [ -z "$GM_LIB" ]; then
    echo "GameMath built but libgm.a was not found under $BUILD_DIR" >&2
    exit 1
fi
echo "library   : $GM_LIB"

# ---------- the run ----------

echo ''
echo 'building...'

# -Wno-macro-redefined: gmath.h redefines isnan, INFINITY and the comparison
# macros that math.h already provides.
run_logged compile.log "$CC_BIN" -O2 -ffp-contract=off -Wno-macro-redefined "$PROGRAM" \
    -I "$SRC_DIR/include" "$GM_LIB" -lm -o "$WORK/bench_game_math"

echo 'timing... (a minute or two; close anything else that wants the CPU)'
run_logged run.log sh -c 'cd "$1" && ./bench_game_math' sh "$WORK"

RESULTS=$(find "$WORK" -maxdepth 1 -type f -name 'bench-*.txt' | sort)
if [ -z "$RESULTS" ]; then
    echo 'the run produced no result files' >&2
    exit 1
fi

# ---------- provenance ----------

# The program knows the platform and the compiler but not which GameMath it was
# linked against, and a file that does not say is worthless a month later: the
# committed macOS numbers were once taken against a Debug build and read three
# times slower than they should have. The line goes in under the title.
describe_machine() {
    printf '%s, macOS %s, %s' \
        "$(sysctl -n machdep.cpu.brand_string)" \
        "$(sw_vers -productVersion)" \
        "$("$CC_BIN" --version | head -1)"
}

annotate() {
    {
        head -1 "$1"
        echo "$(describe_machine), $CC_BIN -O2 -ffp-contract=off"
        echo "GameMath $GM_REV built $CONFIG, intrinsics ${INTRINSICS:-default}."
        tail -n +2 "$1"
    } > "$1.annotated"

    mv -f "$1.annotated" "$1"
}

# ---------- results ----------

echo ''
for RESULT in $RESULTS; do
    NAME=$(basename "$RESULT")
    annotate "$RESULT"
    mv -f "$RESULT" "$OUT_DIR/$NAME"
    echo "wrote $NAME"
done

if [ "$KEEP_EXE" -eq 1 ]; then
    cp -f "$WORK/bench_game_math" "$OUT_DIR/bench_game_math"
    echo "wrote bench_game_math"
fi
