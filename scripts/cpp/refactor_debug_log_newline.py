# Created with python 3.11.4

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
        "REALLY_VERBOSE_LOG",
        "DOUBLE_DEBUG",
        "PERF_LOG",
        "CRCGEN_LOG",
        "STATECHANGED_LOG",
        "PING_LOG",
        "BONEPOS_LOG",
    ]

    PATTERN = r'\n"'
    #PATTERN = r'\r"'
    PATTERN_LEN = len(PATTERN)

    for searchWord in searchWords:
        wordBegin = line.find(searchWord)
        wordEnd = wordBegin + len(searchWord)
        if wordBegin >= 0:
            i = wordEnd
            lookEnd = len(line) - PATTERN_LEN

            while i < lookEnd:
                pattern = line[i:i+PATTERN_LEN]
                if pattern == PATTERN:
                    lineCopy = line[:i] + '"' + line[i+PATTERN_LEN:]
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
