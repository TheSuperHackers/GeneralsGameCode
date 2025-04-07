set(MAPCACHEBUILDER_SRC
    "Include/WinMain.h"
    "Resource/Resource.h"
    "Source/WinMain.cpp"
)

add_executable(z_mapcachebuilder WIN32)
set_target_properties(z_mapcachebuilder PROPERTIES OUTPUT_NAME mapcachebuilder)

target_sources(z_mapcachebuilder PRIVATE ${MAPCACHEBUILDER_SRC})

target_include_directories(z_mapcachebuilder PRIVATE
    Include
    Resource
)
#
target_link_libraries(z_mapcachebuilder PRIVATE
    comctl32
    dbghelplib
    imm32
    vfw32
    winmm
    z_debug
    z_gameengine
    z_gameenginedevice
    z_profile
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(z_mapcachebuilder PRIVATE Resource/MapCacheBuilder.rc)
endif()
