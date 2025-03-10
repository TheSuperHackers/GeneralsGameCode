set(MAPCACHEBUILDER_SRC
    "Include/WinMain.h"
    "Resource/Resource.h"
    "Source/WinMain.cpp"
)

add_executable(mapcachebuilder WIN32)

target_include_directories(mapcachebuilder PRIVATE
    Include
    Resource
    ../../Libraries/Include
)
#
target_link_libraries(mapcachebuilder PRIVATE
    winmm
    imm32
    vfw32
    profile
    dbghelplib
    gameengine
    gameenginedevice
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(mapcachebuilder PRIVATE Resource/MapCacheBuilder.rc)
endif()

target_sources(mapcachebuilder PRIVATE ${MAPCACHEBUILDER_SRC})
