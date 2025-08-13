# Created with python 3.11.4

# This script helps with moving cpp files from Generals or GeneralsMD to Core

import os
import shutil
from enum import Enum


class Game(Enum):
    GENERALS = 0
    ZEROHOUR = 1
    CORE = 2


class CmakeModifyType(Enum):
    ADD_COMMENT = 0
    REMOVE_COMMENT = 1


current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.join(current_dir, "..", "..")
root_dir = os.path.normpath(root_dir)
core_dir = os.path.join(root_dir, "Core")
generals_dir = os.path.join(root_dir, "Generals", "Code")
generalsmd_dir = os.path.join(root_dir, "GeneralsMD", "Code")


def get_game_path(game: Game):
    if game == Game.GENERALS:
        return generals_dir
    elif game == Game.ZEROHOUR:
        return generalsmd_dir
    elif game == Game.CORE:
        return core_dir
    assert(0)


def get_opposite_game(game: Game):
    if game == Game.GENERALS:
        return Game.ZEROHOUR
    elif game == Game.ZEROHOUR:
        return Game.GENERALS
    assert(0)


def move_file(fromGame: Game, fromFile: str, toGame: Game, toFile: str):
    fromPath = os.path.join(get_game_path(fromGame), os.path.normpath(fromFile))
    toPath = os.path.join(get_game_path(toGame), os.path.normpath(toFile))
    os.makedirs(os.path.dirname(toPath), exist_ok=True)
    shutil.move(fromPath, toPath)


def delete_file(game: Game, path: str):
    os.remove(os.path.join(get_game_path(game), os.path.normpath(path)))


def modify_cmakelists(cmakeFile: str, searchString: str, type: CmakeModifyType):
    lines: list[str]
    with open(cmakeFile, 'r', encoding="ascii") as file:
        lines = file.readlines()

    with open(cmakeFile, 'w', encoding="ascii") as file:
        for index, line  in enumerate(lines):
            if searchString in line:
                if type == CmakeModifyType.ADD_COMMENT:
                    lines[index] = "#" + line
                else:
                    lines[index] = line.replace("#", "", 1)

        file.writelines(lines)


def unify_file(fromGame: Game, fromFile: str, toGame: Game, toFile: str):
    assert(toGame == Game.CORE)

    fromOppositeGame = get_opposite_game(fromGame)
    fromOppositeGamePath = get_game_path(fromOppositeGame)
    fromGamePath = get_game_path(fromGame)
    toGamePath = get_game_path(toGame)

    fromFirstFolderIndex = fromFile.find("/")
    toFirstFolderIndex = toFile.find("/")
    assert(fromFirstFolderIndex > 0)
    assert(toFirstFolderIndex > 0)

    fromFirstFolderName = fromFile[:fromFirstFolderIndex]
    toFirstFolderName = toFile[:toFirstFolderIndex]
    fromFileInCmake = fromFile[fromFirstFolderIndex+1:]
    toFileInCmake = toFile[toFirstFolderIndex+1:]

    fromOppositeCmakeFile = os.path.join(fromOppositeGamePath, fromFirstFolderName, "CMakeLists.txt")
    fromCmakeFile = os.path.join(fromGamePath, fromFirstFolderName, "CMakeLists.txt")
    toCmakeFile = os.path.join(toGamePath, toFirstFolderName, "CMakeLists.txt")

    modify_cmakelists(fromOppositeCmakeFile, fromFileInCmake, CmakeModifyType.ADD_COMMENT)
    modify_cmakelists(fromCmakeFile, fromFileInCmake, CmakeModifyType.ADD_COMMENT)
    modify_cmakelists(toCmakeFile, toFileInCmake, CmakeModifyType.REMOVE_COMMENT)

    delete_file(fromOppositeGame, fromFile)
    move_file(fromGame, fromFile, toGame, toFile)


def unify_move_file(fromGame: Game, fromFile: str, toGame: Game, toFile: str):
    assert(toGame == Game.CORE)

    fromGamePath = get_game_path(fromGame)
    toGamePath = get_game_path(toGame)

    fromFirstFolderIndex = fromFile.find("/")
    toFirstFolderIndex = toFile.find("/")
    assert(fromFirstFolderIndex > 0)
    assert(toFirstFolderIndex > 0)

    fromFirstFolderName = fromFile[:fromFirstFolderIndex]
    toFirstFolderName = toFile[:toFirstFolderIndex]
    fromFileInCmake = fromFile[fromFirstFolderIndex+1:]
    toFileInCmake = toFile[toFirstFolderIndex+1:]

    fromCmakeFile = os.path.join(fromGamePath, fromFirstFolderName, "CMakeLists.txt")
    toCmakeFile = os.path.join(toGamePath, toFirstFolderName, "CMakeLists.txt")

    modify_cmakelists(fromCmakeFile, fromFileInCmake, CmakeModifyType.ADD_COMMENT)
    modify_cmakelists(toCmakeFile, toFileInCmake, CmakeModifyType.REMOVE_COMMENT)

    move_file(fromGame, fromFile, toGame, toFile)


def main():

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/crc.h", Game.CORE, "GameEngine/Include/Common/crc.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/CRCDebug.h", Game.CORE, "GameEngine/Include/Common/CRCDebug.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/crc.cpp", Game.CORE, "GameEngine/Source/Common/crc.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/CRCDebug.cpp", Game.CORE, "GameEngine/Source/Common/CRCDebug.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/RandomValue.h", Game.CORE, "GameEngine/Include/Common/RandomValue.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/ClientRandomValue.h", Game.CORE, "GameEngine/Include/GameClient/ClientRandomValue.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameLogic/LogicRandomValue.h", Game.CORE, "GameEngine/Include/GameLogic/LogicRandomValue.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/RandomValue.cpp", Game.CORE, "GameEngine/Source/Common/RandomValue.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/Debug.h", Game.CORE, "GameEngine/Include/Common/Debug.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/Debug.cpp", Game.CORE, "GameEngine/Source/Common/System/Debug.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/VideoPlayer.h", Game.CORE, "GameEngine/Include/GameClient/VideoPlayer.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/VideoPlayer.cpp", Game.CORE, "GameEngine/Source/GameClient/VideoPlayer.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/VideoStream.cpp", Game.CORE, "GameEngine/Source/GameClient/VideoStream.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/WindowVideoManager.h", Game.CORE, "GameEngine/Include/GameClient/WindowVideoManager.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/GUI/WindowVideoManager.cpp", Game.CORE, "GameEngine/Source/GameClient/GUI/WindowVideoManager.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/INI/INIVideo.cpp", Game.CORE, "GameEngine/Source/Common/INI/INIVideo.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/VideoDevice/Bink/BinkVideoPlayer.h", Game.CORE, "GameEngineDevice/Include/VideoDevice/Bink/BinkVideoPlayer.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/VideoDevice/Bink/BinkVideoPlayer.cpp", Game.CORE, "GameEngineDevice/Source/VideoDevice/Bink/BinkVideoPlayer.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DVideoBuffer.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DVideoBuffer.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DVideoBuffer.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DVideoBuffer.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/VideoDevice/FFmpeg/FFmpegFile.h", Game.CORE, "GameEngineDevice/Include/VideoDevice/FFmpeg/FFmpegFile.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/VideoDevice/FFmpeg/FFmpegVideoPlayer.h", Game.CORE, "GameEngineDevice/Include/VideoDevice/FFmpeg/FFmpegVideoPlayer.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/VideoDevice/FFmpeg/FFmpegFile.cpp", Game.CORE, "GameEngineDevice/Source/VideoDevice/FFmpeg/FFmpegFile.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/VideoDevice/FFmpeg/FFmpegVideoPlayer.cpp", Game.CORE, "GameEngineDevice/Source/VideoDevice/FFmpeg/FFmpegVideoPlayer.cpp")

    return


if __name__ == "__main__":
    main()
