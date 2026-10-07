# GameMath benchmark

Times each GameMath function against the system math function it replaces, in double and in float. `Source/bench_game_math.c` writes nanoseconds per call, one file per platform; on 32-bit x86 it measures with the x87 precision control at `_PC_24` and at `_PC_53`. `weigh_bench.sh` and its twin `weigh_bench.ps1` then multiply the timings by how often the game calls each function.

GameMath is the library pinned in `cmake/gamemath.cmake`.

## Run

Give the machine nothing else to do while it runs.

### Windows

Tests and benchmarks together, including the weighting:

```
powershell -ExecutionPolicy Bypass -File scripts\run_all_game_math.ps1
```

Only the benchmark, then the weighting; `-Reverse` measures `_PC_53` before `_PC_24`:

```
powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\run_bench_game_math.ps1
powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\run_bench_game_math.ps1 -Reverse
powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\weigh_bench.ps1
```

### macOS

Tests and benchmarks together, including the weighting:

```
sh scripts/run_all_game_math.sh
```

Only the benchmark, then the weighting:

```
sh Benchmarks/GameMath/Scripts/run_bench_game_math.sh
sh Benchmarks/GameMath/Scripts/weigh_bench.sh
```

`profile_from_counts.sh` and its twin `profile_from_counts.ps1` turn a counter dump from the game into the call profile in `callcounts-zh.txt`. Without a session they list the sessions in the dump:

```
powershell -ExecutionPolicy Bypass -File Benchmarks\GameMath\Scripts\profile_from_counts.ps1 <dump> [session] [first-last]
sh Benchmarks/GameMath/Scripts/profile_from_counts.sh <dump> [session] [first-last]
```

## Results

In `Snapshots/`:

| File | Holds |
| :--- | :--- |
| `bench-<platform>.txt` | nanoseconds per call, GameMath and system, double and float |
| `bench-<platform>-rev.txt` | the same, measured in reverse order |
| `callcounts-zh.txt` | GameMath calls per logic frame in Zero Hour |
| `bench-weighted.txt` | the timings multiplied by the call profile, each route marked with its rows that differ from macOS in `Tests/GameMath/Snapshots/math-summary.txt` |
