set(PRECOMPILED_SRC
    Precompiled/CppMacros.h
    Precompiled/CppTypes.h
    Precompiled/precompiled.h
)

## core_precompiled

add_library(core_precompiled INTERFACE)
target_include_directories(core_precompiled INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(core_precompiled PRIVATE ${PRECOMPILED_SRC})
