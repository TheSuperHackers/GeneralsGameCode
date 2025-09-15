#!/usr/bin/env python3
# TheSuperHackers @build JohnsterID 15/09/2025 Add clang-tidy runner script for code quality analysis

"""
Clang-tidy runner script for GeneralsGameCode project.

This script helps run clang-tidy on the codebase with proper configuration
for the MinGW-w64 cross-compilation environment and legacy C++ code.
"""

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Set


def find_project_root() -> Path:
    """Find the project root directory by looking for CMakeLists.txt."""
    current = Path(__file__).parent
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


def load_compile_commands(compile_commands_path: Path) -> List[dict]:
    """Load and parse the compile_commands.json file."""
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
        
        # Only include C++ source files
        if file_path.suffix in {'.cpp', '.cxx', '.cc', '.c'}:
            source_files.add(str(file_path))
    
    return sorted(source_files)


def run_clang_tidy(source_files: List[str], 
                  compile_commands_path: Path,
                  extra_args: List[str],
                  fix: bool = False,
                  jobs: int = 1) -> int:
    """Run clang-tidy on the specified source files."""
    if not source_files:
        print("No source files to analyze.")
        return 0
    
    cmd = [
        'clang-tidy',
        f'-p={compile_commands_path.parent}',
    ]
    
    if fix:
        cmd.append('--fix')
    
    if extra_args:
        cmd.extend(extra_args)
    
    # Add source files
    cmd.extend(source_files)
    
    print(f"Running clang-tidy on {len(source_files)} files...")
    print(f"Command: {' '.join(cmd[:5])} ... {len(source_files)} files")
    
    try:
        if jobs > 1:
            # For parallel execution, we'd need to use run-clang-tidy.py from LLVM
            # For now, run sequentially
            print(f"Note: Parallel execution with {jobs} jobs not implemented yet.")
        
        result = subprocess.run(cmd, cwd=find_project_root())
        return result.returncode
    except FileNotFoundError:
        print("Error: clang-tidy not found. Please install clang-tidy.")
        return 1
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
        return 130


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
        '--jobs', '-j',
        type=int,
        default=1,
        help='Number of parallel jobs (not implemented yet)'
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
            compile_commands_path,
            args.clang_tidy_args,
            args.fix,
            args.jobs
        )
        
    except RuntimeError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
        return 130


if __name__ == '__main__':
    sys.exit(main())