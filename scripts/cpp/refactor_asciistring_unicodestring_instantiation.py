# Created with python 3.11.4

# This script applies basic formatting and cleanups to the various CPP files.
# Just run it.

import glob
import os
import re


def fix_string(line: str, typename: str) -> str:
    # Build a regex that allows arbitrary whitespace
    pattern = rf"""{typename}\s*\(\s*(L?)\s*"([^"]*)"\s*\)"""
    # Replace typename( "value" ) -> "value"
    return re.sub(pattern, r'\1"\2"', line)


def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.join(current_dir, "..", "..")
    root_dir = os.path.normpath(root_dir)
    core_dir = os.path.join(root_dir, "Core")
    generals_dir = os.path.join(root_dir, "Generals")
    generalsmd_dir = os.path.join(root_dir, "GeneralsMD")
    utility_dir = os.path.join(root_dir, "Dependencies", "Utility")
    fileNames = []
    for ext in ["*.cpp", "*.h", "*.inl"]:
        fileNames.extend(glob.glob(os.path.join(core_dir, '**', ext), recursive=True))
        fileNames.extend(glob.glob(os.path.join(generals_dir, '**', ext), recursive=True))
        fileNames.extend(glob.glob(os.path.join(generalsmd_dir, '**', ext), recursive=True))
        fileNames.extend(glob.glob(os.path.join(utility_dir, '**', ext), recursive=True))

    for fileName in fileNames:
        with open(fileName, 'r', encoding="cp1252") as file:
            try:
                lines = file.readlines()
            except UnicodeDecodeError:
                continue # Not good.

        with open(fileName, 'w', encoding="cp1252") as file:
            for line in lines:
                line = fix_string(line, 'AsciiString')
                line = fix_string(line, 'UnicodeString')
                file.write(line)

    return


if __name__ == "__main__":
    main()
