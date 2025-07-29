# Created with python 3.11.4

# This script helps removing RTS_INTERNAL words from the various CPP files.
# Just run it.

import glob
import os


def modifyLine(line: str) -> str:
    searchWords = [
        "!defined(RTS_DEBUG) && !defined(RTS_INTERNAL)",
        "!defined(RTS_INTERNAL) && !defined(RTS_DEBUG)",
        "defined(RTS_DEBUG) || defined(RTS_INTERNAL)",
        "defined(RTS_INTERNAL) || defined(RTS_DEBUG)",
        "defined( RTS_INTERNAL ) || defined( RTS_DEBUG )",
        "defined RTS_DEBUG || defined RTS_INTERNAL",
        "RTS_DEBUG || RTS_INTERNAL",
    ]

    replaceWords = [
        "!defined(RTS_DEBUG)",
        "!defined(RTS_DEBUG)",
        "defined(RTS_DEBUG)",
        "defined(RTS_DEBUG)",
        "defined(RTS_DEBUG)",
        "defined(RTS_DEBUG)",
        "RTS_DEBUG",
    ]

    for searchIdx, searchWord in enumerate(searchWords):
        wordBegin = line.find(searchWord)
        wordEnd = wordBegin + len(searchWord)
        if wordBegin >= 0:
            replaceWord = replaceWords[searchIdx]
            lineCopy = line[:wordBegin] + replaceWord + line[wordEnd:]
            return lineCopy

    return line


def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.join(current_dir, "..", "..")
    root_dir = os.path.normpath(root_dir)
    core_dir = os.path.join(root_dir, "Core")
    generals_dir = os.path.join(root_dir, "Generals")
    generalsmd_dir = os.path.join(root_dir, "GeneralsMD")
    fileNames = []
    fileNames.extend(glob.glob(os.path.join(core_dir, '**', '*.h'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(core_dir, '**', '*.cpp'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(core_dir, '**', '*.inl'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(generals_dir, '**', '*.h'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(generals_dir, '**', '*.cpp'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(generals_dir, '**', '*.inl'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(generalsmd_dir, '**', '*.h'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(generalsmd_dir, '**', '*.cpp'), recursive=True))
    fileNames.extend(glob.glob(os.path.join(generalsmd_dir, '**', '*.inl'), recursive=True))

    for fileName in fileNames:
        with open(fileName, 'r', encoding="cp1252") as file:
            try:
                lines = file.readlines()
            except UnicodeDecodeError:
                continue # Not good.
        with open(fileName, 'w', encoding="cp1252") as file:
            skipLine = 0
            for line in lines:
                # Skip RTS_INTERNAL ifdef blocks
                if skipLine > 0:
                    if "#if" in line:
                        skipLine += 1
                    elif "#endif" in line:
                        skipLine -= 1
                    continue
                if skipLine > 0:
                    continue
                if line == "#ifdef RTS_INTERNAL\n" or line == "#if defined(RTS_INTERNAL)\n":
                    skipLine += 1
                    continue

                line = modifyLine(line)
                file.write(line)

    return


if __name__ == "__main__":
    main()
