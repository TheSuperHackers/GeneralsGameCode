# Created with python 3.11.4

import glob
import os


def modifyLine(line: str) -> str:
    if 'friend_deleteInstance()' in line:
        return line

    deleteInstanceBegin = line.find('deleteInstance()')
    deleteInstanceEnd = deleteInstanceBegin + len('deleteInstance()')
    if deleteInstanceBegin >= 0:
        i = deleteInstanceBegin

        # Skip MemoryPoolObject::deleteInstance()
        if i >= 2 and line[i-2:i] == '::':
            return line

        # Skip void deleteInstance()
        if i >= 5 and line[i-5:i] == 'void ':
            return line

        # Skip void friend_deleteInstance()
        if i >= 5 and line[i-5:i] == 'void ':
            return line

        # Walk back to object end
        i -= 1
        while i >= 0:
            ch = line[i]
            if ch != '>' and ch != '-' and not ch.isspace():
                break
            i -= 1
        objectEnd = i + 1

        # Walk back to object begin
        while i >= 0:
            ch = line[i]
            if ch.isspace() or ch == '{' or ch == '}':
                break
            i -= 1
        objectBegin = i + 1
        objectName = line[objectBegin:objectEnd]

        if objectName:
            lineCopy = line[:objectBegin]
            lineCopy += f'MemoryPoolObject::deleteInstance({objectName})'
            lineCopy += line[deleteInstanceEnd:]
            return lineCopy
        else:
            lineCopy = line[:deleteInstanceBegin]
            lineCopy += 'MemoryPoolObject::deleteInstance(this)'
            lineCopy += line[deleteInstanceEnd:]
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
            for line in lines:
                line = modifyLine(line)
                file.write(line)

    return


if __name__ == "__main__":
    main()
