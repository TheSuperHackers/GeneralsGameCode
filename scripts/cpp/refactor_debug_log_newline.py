# Created with python 3.11.4

# This script helps removing trailing CR LF characters from game debug log messages in the various CPP files.
# Just run it.

import glob
import os


def modifyLine(line: str) -> str:
    searchWords = [
        "DEBUG_LOG",
        "DEBUG_LOG_LEVEL",
        "DEBUG_CRASH",
        "DEBUG_ASSERTLOG",
        "DEBUG_ASSERTCRASH",
        "RELEASE_CRASH",
        "RELEASE_CRASHLOCALIZED",
        "WWDEBUG_SAY",
        "WWDEBUG_WARNING",
        "WWRELEASE_SAY",
        "WWRELEASE_WARNING",
        "WWRELEASE_ERROR",
        "WWASSERT_PRINT",
        "WWDEBUG_ERROR",
        "SNAPSHOT_SAY",
        "SHATTER_DEBUG_SAY",
        "DBGMSG",
        ### "REALLY_VERBOSE_LOG",
        "DOUBLE_DEBUG",
        "PERF_LOG",
        "CRCGEN_LOG",
        "STATECHANGED_LOG",
        "PING_LOG",
        "BONEPOS_LOG",
    ]

    SEARCH_PATTERNS = [
        r'\r\n"',
        r'\n"',
    ]

    for searchWord in searchWords:
        wordBegin = line.find(searchWord)
        wordEnd = wordBegin + len(searchWord)

        if wordBegin >= 0:
            for searchPattern in SEARCH_PATTERNS:
                searchPatternLen = len(searchPattern)
                i = wordEnd
                lookEnd = len(line) - searchPatternLen

                while i < lookEnd:
                    pattern = line[i:i+searchPatternLen]
                    if pattern == searchPattern:
                        lineCopy = line[:i] + '"' + line[i+searchPatternLen:]
                        return lineCopy
                    i += 1

            break

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
            for line in lines:
                line = modifyLine(line)
                file.write(line)

    return


if __name__ == "__main__":
    main()
