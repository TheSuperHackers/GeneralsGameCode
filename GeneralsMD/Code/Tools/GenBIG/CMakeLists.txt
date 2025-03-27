set(GENBIG_SRC
        "src/BIGFile.h"
        "src/BIGFile.cpp"
        "src/main.h"
        "src/main.cpp"
)

add_executable(z_genbig WIN32)
set_target_properties(z_genbig PROPERTIES OUTPUT_NAME GenBIG)

target_sources(z_genbig PRIVATE ${GENBIG_SRC})

target_include_directories(z_genbig PRIVATE
        src
)

target_link_libraries(z_genbig PRIVATE
        benchmark
        comctl32
        dbghelplib
        imm32
        vfw32
        winmm
        z_debug
        z_gameengine
        z_gameenginedevice
        z_profile
        zi_libraries_include
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_genbig PRIVATE /subsystem:console)
endif()
