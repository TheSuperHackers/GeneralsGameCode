#!/usr/bin/env python3
# TheSuperHackers @build JohnsterID 15/09/2025 Add clang-tidy runner script for code quality analysis
# TheSuperHackers @build bobtista 04/12/2025 Simplify script for PCH-free analysis builds

"""
Clang-tidy runner script for GeneralsGameCode project.

This is a convenience wrapper that:
- Auto-detects the clang-tidy analysis build (build/clang-tidy)
- Filters source files by include/exclude patterns
- Processes files in batches to handle Windows command-line limits
- Provides quiet progress reporting (only shows warnings/errors by default)

For the analysis build to work correctly, it must be built WITHOUT precompiled headers.
Run this first:
  cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja
"""

import argparse
import json
import multiprocessing
import subprocess
import sys
from collections import defaultdict
from pathlib import Path
from typing import List, Optional, Tuple, Dict


def find_clang_tidy() -> str:
    """Find clang-tidy executable in PATH or common locations."""
    try:
        result = subprocess.run(
            ['clang-tidy', '--version'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode == 0:
            return 'clang-tidy'
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass

    import platform
    if platform.system() == 'Darwin':
        import glob
        import re
        possible_paths = glob.glob('/opt/homebrew/Cellar/llvm*/*/bin/clang-tidy')
        possible_paths.extend(glob.glob('/usr/local/Cellar/llvm*/*/bin/clang-tidy'))

        def extract_version(path):
            match = re.search(r'llvm@?(\d+)', path)
            if match:
                return int(match.group(1))
            match = re.search(r'/(\d+)\.(\d+)\.(\d+)', path)
            if match:
                return int(match.group(1)) * 10000 + int(match.group(2)) * 100 + int(match.group(3))
            return 0

        for path in sorted(possible_paths, key=extract_version, reverse=True):
            try:
                result = subprocess.run(
                    [path, '--version'],
                    capture_output=True,
                    text=True,
                    timeout=5
                )
                if result.returncode == 0:
                    return path
            except (FileNotFoundError, subprocess.TimeoutExpired):
                continue

    raise RuntimeError(
        "clang-tidy not found in PATH. Please install clang-tidy:\n"
        "  macOS: brew install llvm\n"
        "  Windows: Install LLVM from https://llvm.org/builds/"
    )


def find_project_root() -> Path:
    """Find the project root directory."""
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / 'CMakeLists.txt').exists():
            return current
        current = current.parent
    raise RuntimeError("Could not find project root (no CMakeLists.txt found)")


def get_clang_tidy_version(clang_tidy_exe: str) -> Optional[str]:
    """Get the version string from clang-tidy."""
    try:
        result = subprocess.run(
            [clang_tidy_exe, '--version'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    return None


def extract_llvm_version(version_string: str) -> Optional[str]:
    """Extract LLVM version number from clang-tidy version string."""
    import re
    patterns = [
        r'LLVM version (\d+\.\d+\.\d+)',
        r'llvm version (\d+\.\d+\.\d+)',
        r'Homebrew LLVM version (\d+\.\d+\.\d+)',
        r'version (\d+\.\d+\.\d+)',
    ]
    for pattern in patterns:
        match = re.search(pattern, version_string, re.IGNORECASE)
        if match:
            return match.group(1)
    return None


def find_clang_tidy_plugin(project_root: Path) -> Optional[str]:
    """Find the GeneralsGameCode clang-tidy plugin."""
    possible_paths = [
        project_root / "scripts" / "clang-tidy-plugin" / "build" / "lib" / "libGeneralsGameCodeClangTidyPlugin.so",
        project_root / "scripts" / "clang-tidy-plugin" / "build" / "lib" / "libGeneralsGameCodeClangTidyPlugin.dylib",
        project_root / "scripts" / "clang-tidy-plugin" / "build" / "lib" / "libGeneralsGameCodeClangTidyPlugin.dll",
        project_root / "scripts" / "clang-tidy-plugin" / "build" / "bin" / "libGeneralsGameCodeClangTidyPlugin.dll",
    ]

    for path in possible_paths:
        if path.exists():
            return str(path)

    return None


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

    clang_tidy_build = project_root / "build" / "clang-tidy"
    compile_commands = clang_tidy_build / "compile_commands.json"

    if not compile_commands.exists():
        raise RuntimeError(
            "compile_commands.json not found!\n\n"
            "Create the analysis build first:\n"
            "  cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja\n\n"
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

        try:
            rel_path = file_path.relative_to(project_root)
        except ValueError:
            continue

        rel_path_str = str(rel_path)

        if include_patterns:
            if not any(pattern in rel_path_str for pattern in include_patterns):
                continue

        if any(pattern in rel_path_str for pattern in exclude_patterns):
            continue

        if file_path.suffix in {'.cpp', '.cxx', '.cc', '.c'}:
            source_files.add(str(file_path))

    return sorted(source_files)


def _run_batch(args: Tuple) -> Tuple[int, Dict[str, List[str]]]:
    """Helper function to run clang-tidy on a batch of files (for multiprocessing)."""
    batch_num, batch, compile_commands_dir, fix, extra_args, project_root, clang_tidy_exe, verbose = args

    cmd = [
        clang_tidy_exe,
        f'-p={compile_commands_dir}',
    ]

    if fix:
        cmd.append('--fix')

    if extra_args:
        cmd.extend(extra_args)

    cmd.extend(batch)

    issues_by_file = defaultdict(list)

    try:
        result = subprocess.run(
            cmd,
            cwd=project_root,
            capture_output=True,
            text=True
        )

        if result.stdout or result.stderr:
            output = result.stdout + result.stderr

            for line in output.splitlines():
                line = line.strip()
                if not line:
                    continue

                line_lower = line.lower()
                is_warning_or_error = ('warning' in line_lower or 'error' in line_lower)

                if ':' in line and (is_warning_or_error or verbose):
                    parts = line.split(':', 1)
                    if parts:
                        file_path = parts[0].strip()
                        if any(file_path.endswith(ext) for ext in ['.cpp', '.cxx', '.cc', '.c', '.h', '.hpp', '.hxx']):
                            try:
                                rel_path = Path(file_path).relative_to(project_root)
                                file_key = str(rel_path)
                            except (ValueError, OSError):
                                file_key = file_path

                            if is_warning_or_error or verbose:
                                issues_by_file[file_key].append(line)

        return result.returncode, dict(issues_by_file)
    except FileNotFoundError:
        error_msg = "Error: clang-tidy not found. Please install LLVM/Clang."
        if verbose:
            print(error_msg, file=sys.stderr)
        return 1, {}


def run_clang_tidy(source_files: List[str],
                  compile_commands_path: Path,
                  extra_args: List[str],
                  fix: bool = False,
                  jobs: int = 1,
                  verbose: bool = False,
                  load_plugin: bool = True) -> int:
    """Run clang-tidy on source files in batches, optionally in parallel."""
    if not source_files:
        print("No source files to analyze.")
        return 0

    clang_tidy_exe = find_clang_tidy()

    project_root = find_project_root()
    plugin_path = None
    if load_plugin:
        plugin_path = find_clang_tidy_plugin(project_root)
        if plugin_path:
            clang_tidy_version_str = get_clang_tidy_version(clang_tidy_exe)
            if clang_tidy_version_str:
                detected_version = extract_llvm_version(clang_tidy_version_str)
                if detected_version:
                    if verbose:
                        print(f"Found clang-tidy plugin: {plugin_path}")
                        print(f"Using clang-tidy LLVM version: {detected_version}")
                        print("Note: Ensure the plugin was built with the same LLVM version (check CMake build output).\n")
                    else:
                        print(f"Note: Verify plugin was built with LLVM {detected_version} (check CMake build output)")
                else:
                    if verbose:
                        print(f"Found clang-tidy plugin: {plugin_path}\n")
            else:
                if verbose:
                    print(f"Found clang-tidy plugin: {plugin_path}\n")

    BATCH_SIZE = 50
    total_files = len(source_files)
    batches = [source_files[i:i + BATCH_SIZE] for i in range(0, total_files, BATCH_SIZE)]

    compile_commands_dir = compile_commands_path.parent

    all_issues = defaultdict(list)
    files_with_issues = set()
    total_issues = 0

    if plugin_path and '-load' not in ' '.join(extra_args):
        extra_args = ['-load', plugin_path] + extra_args

    if jobs > 1:
        if verbose:
            print(f"Running clang-tidy on {total_files} file(s) in {len(batches)} batch(es) with {jobs} workers...\n")
        else:
            print(f"Analyzing {total_files} file(s) with {jobs} workers...", end='', flush=True)

        try:
            with multiprocessing.Pool(processes=jobs) as pool:
                batch_results = pool.map(
                    _run_batch,
                    [
                        (idx + 1, batch, compile_commands_dir, fix, extra_args, project_root, clang_tidy_exe, verbose)
                        for idx, batch in enumerate(batches)
                    ]
                )

            overall_returncode = 0
            for returncode, issues in batch_results:
                if returncode != 0:
                    overall_returncode = returncode
                for file_path, file_issues in issues.items():
                    if file_issues:
                        all_issues[file_path].extend(file_issues)
                        files_with_issues.add(file_path)
                        total_issues += len(file_issues)

            if not verbose:
                print(" done.")
        except KeyboardInterrupt:
            print("\nInterrupted by user.")
            return 130
    else:
        if verbose:
            print(f"Running clang-tidy on {total_files} file(s) in {len(batches)} batch(es)...\n")
        else:
            print(f"Analyzing {total_files} file(s)...", end='', flush=True)

        overall_returncode = 0
        for batch_num, batch in enumerate(batches, 1):
            try:
                if verbose:
                    print(f"Batch {batch_num}/{len(batches)}: {len(batch)} file(s)...")

                returncode, issues = _run_batch((batch_num, batch, compile_commands_dir, fix, extra_args, project_root, clang_tidy_exe, verbose))
                if returncode != 0:
                    overall_returncode = returncode

                for file_path, file_issues in issues.items():
                    if file_issues:
                        all_issues[file_path].extend(file_issues)
                        files_with_issues.add(file_path)
                        total_issues += len(file_issues)

                if not verbose and batch_num < len(batches):
                    print('.', end='', flush=True)
            except KeyboardInterrupt:
                print("\nInterrupted by user.")
                return 130

        if not verbose:
            print(" done.")

    print(f"\nSummary: {len(files_with_issues)} file(s) with issues, {total_issues} total issue(s)")

    if all_issues:
        print("\nIssues found:")
        for file_path in sorted(all_issues.keys()):
            print(f"\n{file_path}:")
            for issue in all_issues[file_path]:
                print(f"  {issue}")

    return overall_returncode


def main():
    parser = argparse.ArgumentParser(
        description="Run clang-tidy on GeneralsGameCode project",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # First-time setup: Create PCH-free analysis build
  cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja

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

  # Show verbose output (default: only warnings/errors)
  python scripts/run-clang-tidy.py --verbose --include Core/Libraries/

  # Use different build directory
  python scripts/run-clang-tidy.py --build-dir build/win32-debug

Note: Requires a PCH-free build. Create with:
      cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja
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
        default=multiprocessing.cpu_count(),
        help=f'Number of parallel workers (default: {multiprocessing.cpu_count()} - auto-detected). Use 1 for serial processing'
    )

    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Show detailed output for each file (default: only show warnings/errors)'
    )

    parser.add_argument(
        '--no-plugin',
        action='store_true',
        help='Do not automatically load the GeneralsGameCode clang-tidy plugin'
    )

    parser.add_argument(
        'clang_tidy_args',
        nargs='*',
        help='Additional arguments to pass to clang-tidy, or specific files to analyze (if files are provided, --include/--exclude are ignored)'
    )

    args = parser.parse_args()

    try:
        compile_commands_path = find_compile_commands(args.build_dir)
        print(f"Using compile commands: {compile_commands_path}\n")

        project_root = find_project_root()
        specified_files = []
        clang_tidy_args = []

        for arg in args.clang_tidy_args:
            file_path = Path(arg)
            if not file_path.is_absolute():
                file_path = project_root / file_path

            if file_path.exists() and file_path.suffix in {'.cpp', '.cxx', '.cc', '.c', '.h', '.hpp'}:
                specified_files.append(str(file_path.resolve()))
            else:
                clang_tidy_args.append(arg)

        if specified_files:
            if args.verbose:
                print(f"Analyzing {len(specified_files)} specified file(s)\n")
            return run_clang_tidy(
                specified_files,
                compile_commands_path,
                clang_tidy_args,
                args.fix,
                args.jobs,
                args.verbose,
                load_plugin=not args.no_plugin
            )

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

        if args.verbose:
            print(f"Found {len(source_files)} source file(s) to analyze\n")

        return run_clang_tidy(
            source_files,
            compile_commands_path,
            clang_tidy_args,
            args.fix,
            args.jobs,
            args.verbose,
            load_plugin=not args.no_plugin
        )

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())

