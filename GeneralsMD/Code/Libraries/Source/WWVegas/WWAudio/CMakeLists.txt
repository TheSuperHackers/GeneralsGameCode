set(WWAUDIO_SRC
    "AABTreeSoundCullClass.h"
    "AudibleSound.cpp"
    "AudibleSound.h"
    "AudioEvents.h"
    "AudioSaveLoad.cpp"
    "AudioSaveLoad.h"
    "FilteredSound.cpp"
    "FilteredSound.h"
    "Listener.cpp"
    "Listener.h"
    "listenerhandle.cpp"
    "listenerhandle.h"
    "LogicalListener.cpp"
    "LogicalListener.h"
    "LogicalSound.cpp"
    "LogicalSound.h"
    "PriorityVector.h"
    "sound2dhandle.cpp"
    "sound2dhandle.h"
    "Sound3D.cpp"
    "Sound3D.h"
    "sound3dhandle.cpp"
    "sound3dhandle.h"
    "SoundBuffer.cpp"
    "SoundBuffer.h"
    "SoundChunkIDs.h"
    "SoundCullObj.h"
    "soundhandle.cpp"
    "soundhandle.h"
    "SoundPseudo3D.cpp"
    "SoundPseudo3D.h"
    "SoundScene.cpp"
    "SoundScene.h"
    "SoundSceneObj.cpp"
    "SoundSceneObj.h"
    "soundstreamhandle.cpp"
    "soundstreamhandle.h"
    "Threads.cpp"
    "Threads.h"
    "Utils.cpp"
    "Utils.h"
    "WWAudio.cpp"
    "WWAudio.h"
)

add_library(z_wwaudio STATIC)
set_target_properties(z_wwaudio PROPERTIES OUTPUT_NAME wwaudio)

target_sources(z_wwaudio PRIVATE ${WWAUDIO_SRC})

target_link_libraries(z_wwaudio PRIVATE
    z_wwcommon
)