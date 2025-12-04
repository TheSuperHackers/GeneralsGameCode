#!/usr/bin/env python3
# TheSuperHackers @build JohnsterID 15/09/2025 Add clang-tidy runner script for code quality analysis
# TheSuperHackers @build bobtista 04/12/2025 Simplify script for PCH-free analysis builds

"""
Clang-tidy runner script for GeneralsGameCode project.

This is a convenience wrapper that:
- Auto-detects the clang-tidy analysis build (build/clang-tidy)
- Filters source files by include/exclude patterns
- Processes files in batches to handle Windows command-line limits
- Provides progress reporting

For the analysis build to work correctly, it must be built WITHOUT precompiled headers.
Run this first:
  cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -G Ninja
"""

import argparse
import json
import multiprocessing
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Tuple


def find_project_root() -> Path:
    """Find the project root directory."""
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / 'CMakeLists.txt').exists():
            return current
        current = current.parent
    raise RuntimeError("Could not find project root (no CMakeLists.txt found)")


def find_compile_commands(build_dir: Optional[Path] = None) -> Path:
    """Find compile_commands.json from the clang-tidy analysis build."""
    project_root = find_project_root()

    if build_dir:
        if not build_dir.is_absolute():
            build_dir = project_root / build_dir
        compile_commands = build_dir / "compile_commands.json"
        if compile_commands.exists():
            return compile_commands
        raise FileNotFoundError(
            f"compile_commands.json not found in {build_dir}"
        )

    # Use the dedicated clang-tidy build (PCH-free, required for correct analysis)
    clang_tidy_build = project_root / "build" / "clang-tidy"
    compile_commands = clang_tidy_build / "compile_commands.json"

    if not compile_commands.exists():
        raise RuntimeError(
            "Clang-tidy build not found!\n\n"
            "Create the analysis build first:\n"
            "  cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -G Ninja\n\n"
            "Or specify a different build with --build-dir"
        )

    return compile_commands


def load_compile_commands(compile_commands_path: Path) -> List[dict]:
    """Load and parse compile_commands.json."""
    try:
        with open(compile_commands_path, 'r') as f:
            return json.load(f)
    except (json.JSONDecodeError, IOError) as e:
        raise RuntimeError(f"Failed to load compile_commands.json: {e}")


def filter_source_files(compile_commands: List[dict],
                       include_patterns: List[str],
                       exclude_patterns: List[str]) -> List[str]:
    """Filter source files based on include/exclude patterns."""
    project_root = find_project_root()
    source_files = set()

    for entry in compile_commands:
        file_path = Path(entry['file'])

        # Convert to relative path for pattern matching
        try:
            rel_path = file_path.relative_to(project_root)
        except ValueError:
            continue  # File outside project root

        rel_path_str = str(rel_path)

        if include_patterns:
            if not any(pattern in rel_path_str for pattern in include_patterns):
                continue

        if any(pattern in rel_path_str for pattern in exclude_patterns):
            continue

        if file_path.suffix in {'.cpp', '.cxx', '.cc', '.c'}:
            source_files.add(str(file_path))

    return sorted(source_files)


def _run_batch(args: Tuple) -> int:
    """Helper function to run clang-tidy on a batch of files (for multiprocessing)."""
    batch_num, batch, compile_commands_dir, fix, extra_args, project_root = args
    
    cmd = [
        'clang-tidy',
        f'-p={compile_commands_dir}',
    ]

    if fix:
        cmd.append('--fix')

    if extra_args:
        cmd.extend(extra_args)

    cmd.extend(batch)

    print(f"Batch {batch_num}: Analyzing {len(batch)} file(s)...")

    try:
        result = subprocess.run(cmd, cwd=project_root)
        return result.returncode
    except FileNotFoundError:
        print("Error: clang-tidy not found. Please install LLVM/Clang.", file=sys.stderr)
        return 1


def run_clang_tidy(source_files: List[str],
                  compile_commands_path: Path,
                  extra_args: List[str],
                  fix: bool = False,
                  jobs: int = 1) -> int:
    """Run clang-tidy on source files in batches, optionally in parallel."""
    if not source_files:
        print("No source files to analyze.")
        return 0

    # Process files in batches (Windows has ~8191 char command-line limit)
    BATCH_SIZE = 50
    total_files = len(source_files)
    batches = [source_files[i:i + BATCH_SIZE] for i in range(0, total_files, BATCH_SIZE)]

    project_root = find_project_root()
    compile_commands_dir = compile_commands_path.parent
    
    if jobs > 1:
        print(f"Running clang-tidy on {total_files} file(s) in {len(batches)} batch(es) with {jobs} workers...\n")

        try:
            with multiprocessing.Pool(processes=jobs) as pool:
                results = pool.map(
                    _run_batch,
                    [
                        (idx + 1, batch, compile_commands_dir, fix, extra_args, project_root)
                        for idx, batch in enumerate(batches)
                    ]
                )
            overall_returncode = max(results) if results else 0
        except KeyboardInterrupt:
            print("\nInterrupted by user.")
            return 130
    else:
        print(f"Running clang-tidy on {total_files} file(s) in {len(batches)} batch(es)...\n")

        overall_returncode = 0
        for batch_num, batch in enumerate(batches, 1):
            try:
                returncode = _run_batch((batch_num, batch, compile_commands_dir, fix, extra_args, project_root))
                if returncode != 0:
                    overall_returncode = returncode
            except KeyboardInterrupt:
                print("\nInterrupted by user.")
                return 130

    return overall_returncode


def main():
    parser = argparse.ArgumentParser(
        description="Run clang-tidy on GeneralsGameCode project",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # First-time setup: Create PCH-free analysis build
  cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -G Ninja

  # Analyze all source files
  python scripts/run-clang-tidy.py

  # Analyze specific directory
  python scripts/run-clang-tidy.py --include Core/Libraries/

  # Analyze with specific checks
  python scripts/run-clang-tidy.py --include GameClient/ -- -checks="-*,modernize-use-nullptr"

  # Apply fixes (use with caution!)
  python scripts/run-clang-tidy.py --fix --include Keyboard.cpp -- -checks="-*,modernize-use-nullptr"

  # Use parallel processing (recommended: --jobs 4 for 6-core CPUs)
  python scripts/run-clang-tidy.py --jobs 4 -- -checks="-*,modernize-use-nullptr"

  # Use different build directory
  python scripts/run-clang-tidy.py --build-dir build/win32-debug

Note: Requires a PCH-free build. Create with:
      cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -G Ninja
        """
    )

    parser.add_argument(
        '--build-dir', '-b',
        type=Path,
        help='Build directory with compile_commands.json (auto-detected if omitted)'
    )

    parser.add_argument(
        '--include', '-i',
        action='append',
        default=[],
        help='Include files matching this pattern (can be used multiple times)'
    )

    parser.add_argument(
        '--exclude', '-e',
        action='append',
        default=[],
        help='Exclude files matching this pattern (can be used multiple times)'
    )

    parser.add_argument(
        '--fix',
        action='store_true',
        help='Apply suggested fixes automatically (use with caution!)'
    )

    parser.add_argument(
        '--jobs', '-j',
        type=int,
        default=1,
        help='Number of parallel workers (default: 1). Recommended: 4 for 6-core CPUs'
    )

    parser.add_argument(
        'clang_tidy_args',
        nargs='*',
        help='Additional arguments to pass to clang-tidy (use after --)'
    )

    args = parser.parse_args()

    try:
        compile_commands_path = find_compile_commands(args.build_dir)
        print(f"Using compile commands: {compile_commands_path}\n")

        compile_commands = load_compile_commands(compile_commands_path)

        default_excludes = [
            'Dependencies/MaxSDK',  # External SDK
            '_deps/',               # CMake dependencies
            'build/',               # Build artifacts
            '.git/',                # Git directory
        ]

        exclude_patterns = default_excludes + args.exclude

        source_files = filter_source_files(
            compile_commands,
            args.include,
            exclude_patterns
        )

        if not source_files:
            print("No source files found matching the criteria.")
            return 1

        print(f"Found {len(source_files)} source file(s) to analyze\n")

        return run_clang_tidy(
            source_files,
            compile_commands_path,
            args.clang_tidy_args,
            args.fix,
            args.jobs
        )

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())

