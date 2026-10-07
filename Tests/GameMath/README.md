# GameMath check

Checks whether GameMath returns the same bits on every platform. `Source/verify_game_math.c` calls each GameMath function on fixed inputs in several ways (double, float, and the casts between them) and writes the raw bits of every result to a dump file, one file per platform. The dumps are then compared line by line against the macOS one.

GameMath is the library pinned in `cmake/gamemath.cmake`.

## Run

Each platform writes its own dumps; commit them on one machine and pull them on the other before comparing.

### Windows

Builds GameMath through the `win32` preset. Tests and benchmarks together:

```
powershell -ExecutionPolicy Bypass -File scripts\run_all_game_math.ps1
```

Only this check, then the comparison:

```
powershell -ExecutionPolicy Bypass -File Tests\GameMath\Scripts\run_verify_game_math.ps1
powershell -ExecutionPolicy Bypass -File Tests\GameMath\Scripts\compare_math.ps1
```

### macOS

Clones GameMath at the pin. Tests and benchmarks together:

```
sh scripts/run_all_game_math.sh
```

Only this check, then the comparison:

```
sh Tests/GameMath/Scripts/run_verify_game_math.sh
sh Tests/GameMath/Scripts/compare_math.sh
```

## Results

In `Snapshots/`:

| File | Holds |
| :--- | :--- |
| `math-<platform>.txt` | the dump of one platform: row, arguments, result bits |
| `math-diff.txt` | a legend of the row names, then the differing lines of each platform against macOS |
| `math-summary.txt` | the number of differing lines per row kind and platform |
