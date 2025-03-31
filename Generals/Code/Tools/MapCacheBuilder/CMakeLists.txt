set(MAPCACHEBUILDER_SRC
    "Include/WinMain.h"
    "Resource/Resource.h"
    "Source/WinMain.cpp"
)

add_executable(g_mapcachebuilder WIN32)
set_target_properties(g_mapcachebuilder PROPERTIES OUTPUT_NAME mapcachebuilder)

target_sources(g_mapcachebuilder PRIVATE ${MAPCACHEBUILDER_SRC})

target_include_directories(g_mapcachebuilder PRIVATE
    Include
    Resource
)
#
target_link_libraries(g_mapcachebuilder PRIVATE
    comctl32
    dbghelplib
    g_gameengine
    g_gameenginedevice
    gi_libraries_include
    imm32
    vfw32
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(g_mapcachebuilder PRIVATE Resource/MapCacheBuilder.rc)
endif()
