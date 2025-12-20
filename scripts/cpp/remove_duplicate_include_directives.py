# Created with python 3.13.2

# This script removes duplicate include directives from the codebase of Generals and GeneralsMD.

import os
import re

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.join(current_dir, "..", "..")
root_dir = os.path.normpath(root_dir)
core_dir = os.path.join(root_dir, "Core")
generals_dir = os.path.join(root_dir, "Generals", "Code")
generalsmd_dir = os.path.join(root_dir, "GeneralsMD", "Code")

def remove_duplicate_includes_from_file(filepath):
    include_pattern = re.compile(r'^\s*#\s*include\s+[<"].+[>"].*$', re.MULTILINE)
    seen = set()
    output_lines = []

    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            match = include_pattern.match(line)
            if match:
                normalized = line.strip()
                if normalized in seen:
                    continue  # Skip duplicate
                seen.add(normalized)
            output_lines.append(line)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(output_lines)

def process_directory(root_dir):
    for subdir, _, files in os.walk(root_dir):
        for file in files:
            if file.endswith(('.cpp', '.h', '.hpp', '.c', '.inl')):
                filepath = os.path.join(subdir, file)
                remove_duplicate_includes_from_file(filepath)

process_directory(generals_dir)
process_directory(generalsmd_dir)
