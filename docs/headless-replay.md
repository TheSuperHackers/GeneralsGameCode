# Headless Replay Mode

## What it is

When `VictoryConditions::update` detects a single remaining
alliance, the engine emits a machine-parseable line to stdout:

```
[HEADLESS RESULT] winner=<internal-player-name>
```

(commit `479554a75`). The internal name is `player0`, `player1`,
... — the slot index, not the human-facing display name — so a CI
/ test harness can extract the winning slot deterministically
regardless of player nicknames.

## Why

Two adjacent use cases:

- **Replay-based regression testing.** When AI changes go in, the
  easiest sanity check is "did this match still end the way the
  recording says it did?" Running `generalszh.exe -file <replay>`
  in headless mode and grepping the stdout for `[HEADLESS
  RESULT]` lets CI assert the winner without any GUI scraping.
- **Tournament archive validation.** Batch-validating uploaded
  replays gives the leaderboard a way to fail fast on
  truncated / corrupted recordings.

## Code surface

Single commit, ~10 lines in
`GeneralsMD/Code/GameEngine/Source/GameLogic/System/VictoryConditions.cpp`.
Inside the existing single-alliance-remaining branch, after the
alliance is identified, emit the result line via the engine's
stdout helper.

## Concerns

- **AI-only setup gap.** As of 2026-05-12 the headless replay path
  is the only end-to-end automation hook; there's no way to set up
  and start an AI-only skirmish from the command line. The
  current workaround is "record manually once, then replay
  headlessly in CI." See memory `project_headless_replay.md`.
- **Mismatch / save / replay.** None. The result line is
  end-of-game-only stdout output, never written to the `.rep`
  file or any save state. Players running interactively see it
  in their debug log if logging is enabled and otherwise don't
  notice it.
- **Output format stability.** The exact string
  `[HEADLESS RESULT] winner=<name>` is now load-bearing for any
  CI scripts that depend on it. Future changes (adding a frame
  count, MVP, etc.) should add fields after `winner=` rather than
  reformatting the leading prefix.
- **Single-winner only.** Mutual-elimination edge cases (everyone
  destroyed in the same frame) don't emit a winner line; the CI
  harness has to treat "no `[HEADLESS RESULT]` line in N seconds"
  as a draw or as an error case explicitly.
