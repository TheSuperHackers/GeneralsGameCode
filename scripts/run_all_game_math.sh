#!/bin/sh
#
# The whole GameMath refresh on macOS in one command: the local
# dump and benchmark, then the comparison of every dump and every benchmark
# result present, the ones pushed from Windows included.
#
#   sh scripts/run_all_game_math.sh [--pull] [runner options]
#
# --pull, when it comes first, runs git pull before anything else. Every other
# option (--rev, --source, --config, --intrinsics, ...) goes to both runners
# unchanged, except --out: the comparison and the weighting read the default
# folders.
#
# Results land in Tests/GameMath/Snapshots and Benchmarks/GameMath/Snapshots,
# wherever this is started from.

set -e

if [ "$(uname -s)" != Darwin ]; then
    echo "only macOS builds are supported" >&2
    exit 1
fi

REPO_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TEST_SCRIPTS=$REPO_ROOT/Tests/GameMath/Scripts
BENCH_SCRIPTS=$REPO_ROOT/Benchmarks/GameMath/Scripts

for ARG in "$@"; do
    if [ "$ARG" = --out ]; then
        echo "--out is not supported here: the comparison and the weighting read the default folders" >&2
        exit 2
    fi
done

if [ "$1" = --pull ]; then
    shift
    echo '=== git pull ==='
    git -C "$REPO_ROOT" pull
    echo ''
fi

echo '=== verify ==='
sh "$TEST_SCRIPTS/run_verify_game_math.sh" "$@"

echo ''
echo '=== bench ==='
sh "$BENCH_SCRIPTS/run_bench_game_math.sh" "$@"

echo ''
echo '=== compare dumps ==='
sh "$TEST_SCRIPTS/compare_math.sh"

echo ''
echo '=== weigh benchmarks ==='
sh "$BENCH_SCRIPTS/weigh_bench.sh"

echo ''
echo 'done; dumps and their comparison are in Tests/GameMath/Snapshots, timings and their weighting in Benchmarks/GameMath/Snapshots'
