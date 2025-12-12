if (NOT IS_VS6_BUILD)
    add_executable(z_datachunkconverter)
    set_target_properties(z_datachunkconverter PROPERTIES OUTPUT_NAME datachunkconverter WIN32_EXECUTABLE FALSE)

    target_sources(z_datachunkconverter PRIVATE
        DataChunkConverter.cpp
    )

    target_include_directories(z_datachunkconverter PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../GameEngine/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../GameEngine/Include/Precompiled
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Include
        ${CMAKE_SOURCE_DIR}/Core/GameEngine/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../GameEngineDevice/Include
    )

    target_compile_definitions(z_datachunkconverter PRIVATE
        RTS_HAS_JSON_CHUNK
    )

    target_link_libraries(z_datachunkconverter PRIVATE
        z_gameengine
        z_gameenginedevice
        zi_always
        comctl32
        core_debug
        core_profile
        winmm
        vfw32
        imm32
    )
endif()
