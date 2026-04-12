# Created with python 3.11.4

# This script aims to find and remove superfluous trailing return words in functions.
# Just run it.

import glob
import os


def apply_fix(line: str, nextLine: str) -> str:
    lineStripped = line.strip()
    if (lineStripped.find("return;") == 0 or
        lineStripped.find("return ;") == 0):
        if nextLine.find("}") == 0:
            return ""

    return line


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
            newLines = []
            for index,line in enumerate(lines):
                if index+1 < len(lines):
                    nextLineIndex = index + 1
                    nextLine = lines[nextLineIndex]
                    while (nextLine.isspace() or nextLine == "") and nextLineIndex+1 < len(lines):
                        nextLineIndex += 1
                        nextLine = lines[nextLineIndex]

                    line = apply_fix(line, nextLine)

                    if line == "":
                        while (newLines and newLines[-1].isspace()) or (newLines and newLines[-1] == ""):
                            newLines.pop()

                newLines.append(line)

            file.writelines(newLines)

    return


if __name__ == "__main__":
    main()
