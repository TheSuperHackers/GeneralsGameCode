# Created with python 3.11.4

# This script applies basic formatting and cleanups to the various CPP files.
# Just run it.

import glob
import os


def apply_formatting(line: str) -> str:
    # Strip useless comments behind scope end
    scopeEndIndex = line.find('}')
    if scopeEndIndex >= 0:
        if scopeEndIndex == 0 or line[:scopeEndIndex].isspace():
            afterScopeIndex = scopeEndIndex + 1
            while afterScopeIndex < len(line) and line[afterScopeIndex] == ';':
                afterScopeIndex += 1
            commentBeginIndex = line.find("//")
            namespaceCommentBeginIndex = line.find("// namespace")
            if commentBeginIndex >= 0 and namespaceCommentBeginIndex < 0:
                if afterScopeIndex == commentBeginIndex or line[afterScopeIndex:commentBeginIndex].isspace():
                    line = line[:commentBeginIndex]

    # remove trailing whitespace
    line = line.rstrip()
    line += "\n"

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
            for line in lines:
                line = apply_formatting(line)
                file.write(line)
            if lines:
                lastLine = lines[-1]
                if lastLine and lastLine[-1] != '\n':
                    file.write("\n") # write new line to end of file

    return


if __name__ == "__main__":
    main()
