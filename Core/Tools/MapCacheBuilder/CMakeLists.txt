set(MAPCACHEBUILDER_SRC
    "Include/WinMain.h"
    "Resource/Resource.h"
    "Source/WinMain.cpp"
)

add_library(corei_mapcachebuilder INTERFACE)

target_sources(corei_mapcachebuilder INTERFACE ${MAPCACHEBUILDER_SRC})

target_include_directories(corei_mapcachebuilder INTERFACE
    Include
    Resource
)
#
target_link_libraries(corei_mapcachebuilder INTERFACE
    comctl32
    dbghelplib
    imm32
    vfw32
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(corei_mapcachebuilder INTERFACE Resource/MapCacheBuilder.rc)
endif()
