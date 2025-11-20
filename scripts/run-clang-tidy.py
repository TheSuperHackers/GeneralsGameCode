#!/usr/bin/env python3
# TheSuperHackers @build bobtista 17/11/2025 Add clang-tidy runner script for code quality analysis

"""
Clang-tidy runner script for GeneralsGameCode project.

This script helps run clang-tidy on the codebase with proper configuration
for the MinGW-w64 cross-compilation environment and legacy C++ code.
"""

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import List, Optional, Set


def find_clang_tidy() -> str:
    """Find clang-tidy executable, checking common locations."""
    # Check common locations
    candidates = ['clang-tidy']

    system = platform.system()

    # On macOS with Homebrew, check the Homebrew LLVM path
    if system == 'Darwin':
        homebrew_llvm = Path('/opt/homebrew/opt/llvm/bin/clang-tidy')
        if homebrew_llvm.exists():
            candidates.insert(0, str(homebrew_llvm))
        # Also check Intel Mac location
        intel_llvm = Path('/usr/local/opt/llvm/bin/clang-tidy')
        if intel_llvm.exists():
            candidates.insert(0, str(intel_llvm))
    # On Windows, check common LLVM installation paths
    elif system == 'Windows':
        # Check Program Files locations (common LLVM installation paths)
        program_files = os.environ.get('ProgramFiles', 'C:\\Program Files')
        program_files_x86 = os.environ.get('ProgramFiles(x86)', 'C:\\Program Files (x86)')

        # Check both 64-bit and 32-bit Program Files
        for pf in [program_files, program_files_x86]:
            llvm_path = Path(pf) / 'LLVM' / 'bin' / 'clang-tidy.exe'
            if llvm_path.exists():
                candidates.insert(0, str(llvm_path))

        # Also check if Visual Studio's clang-tidy is available
        vs_install_dir = os.environ.get('VSINSTALLDIR', '')
        if vs_install_dir:
            vs_clang_tidy = Path(vs_install_dir) / 'VC' / 'Tools' / 'Llvm' / 'bin' / 'clang-tidy.exe'
            if vs_clang_tidy.exists():
                candidates.insert(0, str(vs_clang_tidy))

    # Try each candidate
    for candidate in candidates:
        try:
            result = subprocess.run(
                [candidate, '--version'],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode == 0:
                return candidate
        except (FileNotFoundError, subprocess.TimeoutExpired):
            continue

    raise RuntimeError(
        "clang-tidy not found. Please install clang-tidy:\n"
        "  macOS: brew install llvm\n"
        "  Linux: apt install clang-tidy or yum install clang-tools-extra\n"
        "  Windows: Install LLVM from https://llvm.org/builds/ or use Visual Studio's clang-tidy"
    )


def find_project_root() -> Path:
    """Find the project root directory by looking for CMakeLists.txt."""
    current = Path(__file__).parent.resolve()
    while current != current.parent:
        if (current / "CMakeLists.txt").exists():
            return current
        current = current.parent
    raise RuntimeError("Could not find project root (CMakeLists.txt not found)")


def find_compile_commands(build_dir: Optional[Path] = None) -> Path:
    """Find the compile_commands.json file."""
    project_root = find_project_root()

    if build_dir:
        compile_commands = build_dir / "compile_commands.json"
        if compile_commands.exists():
            return compile_commands

    # Search common build directories
    build_dirs = [
        project_root / "build",
        project_root / "build" / "test",
        project_root / "build" / "win32",
        project_root / "build" / "vc6",
        project_root / "build" / "unix",
    ]

    for build_path in build_dirs:
        compile_commands = build_path / "compile_commands.json"
        if compile_commands.exists():
            return compile_commands

    raise RuntimeError(
        "Could not find compile_commands.json. "
        "Please run cmake with CMAKE_EXPORT_COMPILE_COMMANDS=ON first."
    )


def sanitize_compile_command(entry: dict) -> dict:
    """Remove PCH and other incompatible flags for clang-tidy."""
    cmd = entry.get('command', '')

    # Remove MSVC precompiled header flags that clang-tidy can't handle
    # Note: Order matters - remove complex patterns first
    flags_to_remove = [
        # Clang/GCC PCH flags (must come before simple patterns)
        r'-Xclang\s+-include-pch\s+-Xclang\s+[^\s]+',  # -Xclang -include-pch -Xclang <path>
        r'-Xclang\s+-include-pch\s+[^\s]+',  # -Xclang -include-pch <path>
        r'-Xclang\s+-emit-pch[^\s]*(?:\s+[^\s]+)*',  # -Xclang -emit-pch ... (PCH creation)
        r'-include-pch\s+[^\s]+',  # -include-pch <path>
        r'-Winvalid-pch\b',  # -Winvalid-pch flag
        r'cmake_pch\.hxx\.pch',  # PCH file references
        r'cmake_pch\.hxx\b',  # PCH header references (standalone)

        # MSVC precompiled header flags
        r'/Yu[^\s]*',  # MSVC: Use precompiled header
        r'/Yc[^\s]*',  # MSVC: Create precompiled header
        r'/Fp[^\s]*',  # MSVC: Precompiled header file path
        r'/FI[^\s]*',  # MSVC: Force include file (used for PCH)
    ]

    for flag_pattern in flags_to_remove:
        cmd = re.sub(flag_pattern, '', cmd)

    # Clean up multiple spaces that might result from flag removal
    cmd = re.sub(r'\s+', ' ', cmd).strip()

    entry['command'] = cmd
    return entry


def load_compile_commands(compile_commands_path: Path) -> List[dict]:
    """Load and parse the compile_commands.json file."""
    try:
        with open(compile_commands_path, 'r') as f:
            commands = json.load(f)
        # Sanitize commands to remove PCH flags
        return [sanitize_compile_command(cmd) for cmd in commands]
    except (json.JSONDecodeError, IOError) as e:
        raise RuntimeError(f"Failed to load compile_commands.json: {e}")


def filter_source_files(compile_commands: List[dict],
                       include_patterns: List[str],
                       exclude_patterns: List[str]) -> List[str]:
    """Filter source files based on include/exclude patterns."""
    project_root = find_project_root()
    source_files = set()  # Use set to deduplicate files

    for entry in compile_commands:
        file_path = Path(entry['file'])

        # Skip PCH creation files (cmake_pch.hxx.cxx)
        if 'cmake_pch.hxx.cxx' in str(file_path):
            continue

        # Convert to relative path for pattern matching
        try:
            rel_path = file_path.relative_to(project_root)
        except ValueError:
            # File is outside project root, skip
            continue

        rel_path_str = str(rel_path)

        # Check include patterns
        if include_patterns:
            if not any(pattern in rel_path_str for pattern in include_patterns):
                continue

        # Check exclude patterns
        if any(pattern in rel_path_str for pattern in exclude_patterns):
            continue

        # Only include C++ source files (not headers, not PCH files)
        if file_path.suffix in {'.cpp', '.cxx', '.cc', '.c'}:
            # Use absolute path as string for deduplication
            source_files.add(str(file_path.resolve()))

    return sorted(source_files)


def create_sanitized_compile_commands(compile_commands: List[dict],
                                     original_path: Path,
                                     source_files: List[str]) -> Path:
    """Create a temporary sanitized compile_commands.json file.

    Only includes entries for the source files we want to analyze, and deduplicates
    so each file appears only once.

    Returns the path to the directory containing the sanitized compile_commands.json.
    """
    # Create a temporary directory for the sanitized compile commands
    # We need the file to be named exactly "compile_commands.json" for clang-tidy -p
    temp_dir = Path(tempfile.mkdtemp(
        suffix='_clang_tidy',
        prefix='sanitized_',
        dir=original_path.parent
    ))

    temp_file = temp_dir / 'compile_commands.json'

    # Convert source_files to a set of absolute paths for fast lookup
    source_files_set = {str(Path(f).resolve()) for f in source_files}

    # Deduplicate: keep only one compile command per source file
    seen_files = set()
    deduplicated_commands = []

    for entry in compile_commands:
        file_path = Path(entry['file'])
        abs_file_path = str(file_path.resolve())

        # Only include commands for files we want to analyze
        if abs_file_path not in source_files_set:
            continue

        # Deduplicate: use first occurrence of each file
        if abs_file_path not in seen_files:
            seen_files.add(abs_file_path)
            deduplicated_commands.append(entry)

    try:
        with open(temp_file, 'w') as f:
            json.dump(deduplicated_commands, f, indent=2)
        return temp_dir
    except Exception as e:
        # Clean up on error
        try:
            import shutil
            shutil.rmtree(temp_dir)
        except:
            pass
        raise RuntimeError(f"Failed to create sanitized compile commands: {e}")


def run_clang_tidy(source_files: List[str],
                  compile_commands: List[dict],
                  compile_commands_path: Path,
                  extra_args: List[str],
                  fix: bool = False) -> int:
    """Run clang-tidy on the specified source files."""
    if not source_files:
        print("No source files to analyze.")
        return 0

    # Create a temporary sanitized compile_commands.json
    print("Creating sanitized compile commands...")
    temp_compile_commands = create_sanitized_compile_commands(
        compile_commands,
        compile_commands_path,
        source_files
    )

    try:
        # Process files in batches to avoid command line length limits on Windows
        # Windows cmd.exe has a limit of ~8191 characters
        BATCH_SIZE = 50  # Conservative batch size for Windows compatibility
        total_files = len(source_files)
        batches = [source_files[i:i + BATCH_SIZE] for i in range(0, total_files, BATCH_SIZE)]

        print(f"Running clang-tidy on {total_files} files in {len(batches)} batch(es)...")

        # Find clang-tidy executable
        clang_tidy_exe = find_clang_tidy()

        overall_returncode = 0
        for batch_num, batch in enumerate(batches, 1):
            cmd = [
                clang_tidy_exe,
                f'-p={temp_compile_commands}',
            ]

            if fix:
                cmd.append('--fix')

            if extra_args:
                cmd.extend(extra_args)

            # Add source files for this batch
            cmd.extend(batch)

            print(f"\nBatch {batch_num}/{len(batches)}: Analyzing {len(batch)} file(s)...")

            try:
                result = subprocess.run(cmd, cwd=find_project_root())
                if result.returncode != 0:
                    overall_returncode = result.returncode
            except FileNotFoundError:
                print(f"Error: clang-tidy not found at {clang_tidy_exe}")
                return 1
            except KeyboardInterrupt:
                print("\nInterrupted by user.")
                return 130

        return overall_returncode

    finally:
        # Clean up the temporary directory
        try:
            shutil.rmtree(temp_compile_commands)
            print(f"Cleaned up temporary directory: {temp_compile_commands.name}")
        except Exception as e:
            print(f"Warning: Could not remove temporary directory {temp_compile_commands}: {e}")


def main():
    parser = argparse.ArgumentParser(
        description="Run clang-tidy on GeneralsGameCode project",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Analyze all source files
  python3 scripts/run-clang-tidy.py

  # Analyze only Core directory
  python3 scripts/run-clang-tidy.py --include Core/

  # Analyze GeneralsMD but exclude tests
  python3 scripts/run-clang-tidy.py --include GeneralsMD/ --exclude test

  # Fix issues automatically (use with caution!)
  python3 scripts/run-clang-tidy.py --fix --include Core/Libraries/

  # Use specific build directory
  python3 scripts/run-clang-tidy.py --build-dir build/win32
        """
    )

    parser.add_argument(
        '--build-dir', '-b',
        type=Path,
        help='Build directory containing compile_commands.json'
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
        'clang_tidy_args',
        nargs='*',
        help='Additional arguments to pass to clang-tidy'
    )

    args = parser.parse_args()

    try:
        # Find compile commands
        compile_commands_path = find_compile_commands(args.build_dir)
        print(f"Using compile commands: {compile_commands_path}")

        # Load compile commands
        compile_commands = load_compile_commands(compile_commands_path)
        print(f"Loaded {len(compile_commands)} compile commands")

        # Default exclude patterns for this project
        default_excludes = [
            'Dependencies/MaxSDK',  # External SDK
            '_deps/',               # CMake dependencies
            'build/',               # Build artifacts
            '.git/',                # Git directory
            'stlport.diff',         # Patch file
        ]

        exclude_patterns = default_excludes + args.exclude

        # Filter source files
        source_files = filter_source_files(
            compile_commands,
            args.include,
            exclude_patterns
        )

        if not source_files:
            print("No source files found matching the criteria.")
            return 1

        print(f"Found {len(source_files)} source files to analyze")

        # Run clang-tidy
        return run_clang_tidy(
            source_files,
            compile_commands,
            compile_commands_path,
            args.clang_tidy_args,
            args.fix
        )

    except RuntimeError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
        return 130


if __name__ == '__main__':
    sys.exit(main())