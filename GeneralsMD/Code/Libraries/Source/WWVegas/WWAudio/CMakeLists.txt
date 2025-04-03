set(WWAUDIO_SRC
    "AudibleSound.cpp"
    "AudioSaveLoad.cpp"
    "FilteredSound.cpp"
    "Listener.cpp"
    "listenerhandle.cpp"
    "LogicalListener.cpp"
    "LogicalSound.cpp"
    "sound2dhandle.cpp"
    "Sound3D.cpp"
    "sound3dhandle.cpp"
    "SoundBuffer.cpp"
    "soundhandle.cpp"
    "SoundPseudo3D.cpp"
    "SoundScene.cpp"
    "SoundSceneObj.cpp"
    "soundstreamhandle.cpp"
    "Threads.cpp"
    "Utils.cpp"
    "WWAudio.cpp"
)

add_library(z_wwaudio STATIC)
set_target_properties(z_wwaudio PROPERTIES OUTPUT_NAME wwaudio)

target_sources(z_wwaudio PRIVATE ${WWAUDIO_SRC})

target_link_libraries(z_wwaudio PRIVATE
    z_wwcommon
)