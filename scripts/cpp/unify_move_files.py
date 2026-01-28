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

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/GameMemory.h", Game.CORE, "GameEngine/Include/Common/GameMemory.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/GameMemoryNull.h", Game.CORE, "GameEngine/Include/Common/GameMemoryNull.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/GameMemory.cpp", Game.CORE, "GameEngine/Source/Common/System/GameMemory.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/GameMemoryNull.cpp", Game.CORE, "GameEngine/Source/Common/System/GameMemoryNull.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/MemoryInit.cpp", Game.CORE, "GameEngine/Source/Common/System/GameMemoryInit.cpp")
    #unify_move_file(Game.GENERALS, "GameEngine/Source/Common/System/GameMemoryInitDMA_Generals.inl", Game.CORE, "GameEngine/Source/Common/System/GameMemoryInitDMA_Generals.inl")
    #unify_move_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/GameMemoryInitDMA_GeneralsMD.inl", Game.CORE, "GameEngine/Source/Common/System/GameMemoryInitDMA_GeneralsMD.inl")
    #unify_move_file(Game.GENERALS, "GameEngine/Source/Common/System/GameMemoryInitPools_Generals.inl", Game.CORE, "GameEngine/Source/Common/System/GameMemoryInitPools_Generals.inl")
    #unify_move_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/GameMemoryInitPools_GeneralsMD.inl", Game.CORE, "GameEngine/Source/Common/System/GameMemoryInitPools_GeneralsMD.inl")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/ObjectStatusTypes.h", Game.CORE, "GameEngine/Include/Common/ObjectStatusTypes.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/Radar.h", Game.CORE, "GameEngine/Include/Common/Radar.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/ObjectStatusTypes.cpp", Game.CORE, "GameEngine/Source/Common/System/ObjectStatusTypes.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/Radar.cpp", Game.CORE, "GameEngine/Source/Common/System/Radar.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/Common/W3DRadar.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/Common/W3DRadar.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/Common/System/W3DRadar.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/Common/System/W3DRadar.cpp")

    #unify_move_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/Smudge.h", Game.CORE, "GameEngine/Include/GameClient/Smudge.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/System/Smudge.cpp", Game.CORE, "GameEngine/Source/GameClient/System/Smudge.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DSmudge.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DSmudge.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DSmudge.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DSmudge.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DShaderManager.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DShaderManager.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DShaderManager.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DShaderManager.cpp")

    #unify_move_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/ParabolicEase.h", Game.CORE, "GameEngine/Include/GameClient/ParabolicEase.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/ParabolicEase.cpp", Game.CORE, "GameEngine/Source/GameClient/ParabolicEase.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/camerashakesystem.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/CameraShakeSystem.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/camerashakesystem.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/CameraShakeSystem.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/View.h", Game.CORE, "GameEngine/Include/GameClient/View.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/View.cpp", Game.CORE, "GameEngine/Source/GameClient/View.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DView.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DView.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp")

    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/bmp2d.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/bmp2d.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/bmp2d.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/bmp2d.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/dx8texman.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/dx8texman.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/dx8texman.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/dx8texman.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/matpass.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/matpass.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/matpass.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/matpass.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texproject.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texproject.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texproject.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texproject.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texture.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texture.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texture.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texture.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texturefilter.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texturefilter.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texturefilter.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texturefilter.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/textureloader.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/textureloader.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/textureloader.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/textureloader.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texturethumbnail.cpp", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texturethumbnail.cpp")
    #unify_file(Game.ZEROHOUR, "Libraries/Source/WWVegas/WW3D2/texturethumbnail.h", Game.CORE, "Libraries/Source/WWVegas/WW3D2/texturethumbnail.h")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/Water.h", Game.CORE, "GameEngine/Include/GameClient/Water.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/Water.cpp", Game.CORE, "GameEngine/Source/GameClient/Water.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DLaserDraw.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DLaserDraw.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DWater.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DWater.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DWaterTracks.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DWaterTracks.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DLaserDraw.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DLaserDraw.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWaterTracks.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWaterTracks.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Water/wave.nvp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Water/wave.nvp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Water/wave.nvv", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Water/wave.nvv")

    #unify_move_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/Snow.h", Game.CORE, "GameEngine/Include/GameClient/Snow.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/Snow.cpp", Game.CORE, "GameEngine/Source/GameClient/Snow.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/BaseHeightMap.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/BaseHeightMap.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/FlatHeightMap.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/FlatHeightMap.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DPropBuffer.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DPropBuffer.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DSnow.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DSnow.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainBackground.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainBackground.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DPropDraw.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DPropDraw.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DTreeDraw.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DTreeDraw.h")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/FlatHeightMap.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/FlatHeightMap.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DPropBuffer.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DPropBuffer.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DSnow.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DSnow.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainBackground.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainBackground.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DPropDraw.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DPropDraw.cpp")
    #unify_move_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTreeDraw.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTreeDraw.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/MapObject.h", Game.CORE, "GameEngine/Include/Common/MapObject.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/MapUtil.h", Game.CORE, "GameEngine/Include/GameClient/MapUtil.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/TerrainRoads.h", Game.CORE, "GameEngine/Include/GameClient/TerrainRoads.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/TerrainVisual.h", Game.CORE, "GameEngine/Include/GameClient/TerrainVisual.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/MapUtil.cpp", Game.CORE, "GameEngine/Source/GameClient/MapUtil.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/Terrain/TerrainRoads.cpp", Game.CORE, "GameEngine/Source/GameClient/Terrain/TerrainRoads.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/Terrain/TerrainVisual.cpp", Game.CORE, "GameEngine/Source/GameClient/Terrain/TerrainVisual.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/HeightMap.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/HeightMap.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/TerrainTex.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/TerrainTex.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/TileData.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/TileData.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainTracks.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainTracks.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainVisual.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainVisual.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTreeBuffer.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/W3DTreeBuffer.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Include/W3DDevice/GameClient/WorldHeightMap.h", Game.CORE, "GameEngineDevice/Include/W3DDevice/GameClient/WorldHeightMap.h")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/TerrainTex.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/TerrainTex.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/TileData.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/TileData.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainTracks.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainTracks.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainVisual.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainVisual.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTreeBuffer.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/W3DTreeBuffer.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp", Game.CORE, "GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/UserPreferences.h", Game.CORE, "GameEngine/Include/Common/UserPreferences.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/UserPreferences.cpp", Game.CORE, "GameEngine/Source/Common/UserPreferences.cpp")

    #unify_file(Game.ZEROHOUR, "Libraries/Include/Lib/BaseType.h", Game.CORE, "Libraries/Include/Lib/BaseType.h")
    #unify_file(Game.ZEROHOUR, "Libraries/Include/Lib/trig.h", Game.CORE, "Libraries/Include/Lib/trig.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/Errors.h", Game.CORE, "GameEngine/Include/Common/Errors.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/GameCommon.h", Game.CORE, "GameEngine/Include/Common/GameCommon.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/GameType.h", Game.CORE, "GameEngine/Include/Common/GameType.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/INI.h", Game.CORE, "GameEngine/Include/Common/INI.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/Snapshot.h", Game.CORE, "GameEngine/Include/Common/Snapshot.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/STLTypedefs.h", Game.CORE, "GameEngine/Include/Common/STLTypedefs.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/Common/SubsystemInterface.h", Game.CORE, "GameEngine/Include/Common/SubsystemInterface.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/ChallengeGenerals.h", Game.CORE, "GameEngine/Include/GameClient/ChallengeGenerals.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/INI/INI.cpp", Game.CORE, "GameEngine/Source/Common/INI/INI.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/GameCommon.cpp", Game.CORE, "GameEngine/Source/Common/System/GameCommon.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/GameType.cpp", Game.CORE, "GameEngine/Source/Common/System/GameType.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/Snapshot.cpp", Game.CORE, "GameEngine/Source/Common/System/Snapshot.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/Common/System/SubsystemInterface.cpp", Game.CORE, "GameEngine/Source/Common/System/SubsystemInterface.cpp")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/GUI/ChallengeGenerals.cpp", Game.CORE, "GameEngine/Source/GameClient/GUI/ChallengeGenerals.cpp")

    #unify_file(Game.ZEROHOUR, "GameEngine/Include/GameClient/ParticleSys.h", Game.CORE, "GameEngine/Include/GameClient/ParticleSys.h")
    #unify_file(Game.ZEROHOUR, "GameEngine/Source/GameClient/System/ParticleSys.cpp", Game.CORE, "GameEngine/Source/GameClient/System/ParticleSys.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/res/ParticleEditor.rc2", Game.CORE, "Tools/ParticleEditor/res/ParticleEditor.rc2")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CButtonShowColor.cpp", Game.CORE, "Tools/ParticleEditor/CButtonShowColor.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CButtonShowColor.h", Game.CORE, "Tools/ParticleEditor/CButtonShowColor.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CColorAlphaDialog.cpp", Game.CORE, "Tools/ParticleEditor/CColorAlphaDialog.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CColorAlphaDialog.h", Game.CORE, "Tools/ParticleEditor/CColorAlphaDialog.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CParticleEditorPage.h", Game.CORE, "Tools/ParticleEditor/CParticleEditorPage.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CSwitchesDialog.cpp", Game.CORE, "Tools/ParticleEditor/CSwitchesDialog.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/CSwitchesDialog.h", Game.CORE, "Tools/ParticleEditor/CSwitchesDialog.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/EmissionTypePanels.cpp", Game.CORE, "Tools/ParticleEditor/EmissionTypePanels.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/EmissionTypePanels.h", Game.CORE, "Tools/ParticleEditor/EmissionTypePanels.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ISwapablePanel.h", Game.CORE, "Tools/ParticleEditor/ISwapablePanel.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/MoreParmsDialog.cpp", Game.CORE, "Tools/ParticleEditor/MoreParmsDialog.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/MoreParmsDialog.h", Game.CORE, "Tools/ParticleEditor/MoreParmsDialog.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditor.cpp", Game.CORE, "Tools/ParticleEditor/ParticleEditor.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditor.def", Game.CORE, "Tools/ParticleEditor/ParticleEditor.def")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditor.h", Game.CORE, "Tools/ParticleEditor/ParticleEditor.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditor.rc", Game.CORE, "Tools/ParticleEditor/ParticleEditor.rc")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditorDialog.cpp", Game.CORE, "Tools/ParticleEditor/ParticleEditorDialog.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditorDialog.h", Game.CORE, "Tools/ParticleEditor/ParticleEditorDialog.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleEditorExport.h", Game.CORE, "Tools/ParticleEditor/ParticleEditorExport.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleTypePanels.cpp", Game.CORE, "Tools/ParticleEditor/ParticleTypePanels.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ParticleTypePanels.h", Game.CORE, "Tools/ParticleEditor/ParticleTypePanels.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/post-build-Release.bat", Game.CORE, "Tools/ParticleEditor/post-build-Release.bat")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/post-build.bat", Game.CORE, "Tools/ParticleEditor/post-build.bat")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/Resource.h", Game.CORE, "Tools/ParticleEditor/Resource.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ShaderTypePanels.cpp", Game.CORE, "Tools/ParticleEditor/ShaderTypePanels.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/ShaderTypePanels.h", Game.CORE, "Tools/ParticleEditor/ShaderTypePanels.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/StdAfx.cpp", Game.CORE, "Tools/ParticleEditor/StdAfx.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/StdAfx.h", Game.CORE, "Tools/ParticleEditor/StdAfx.h")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/VelocityTypePanels.cpp", Game.CORE, "Tools/ParticleEditor/VelocityTypePanels.cpp")
    #unify_file(Game.ZEROHOUR, "Tools/ParticleEditor/VelocityTypePanels.h", Game.CORE, "Tools/ParticleEditor/VelocityTypePanels.h")

    return


if __name__ == "__main__":
    main()
