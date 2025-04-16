set(WDUMP_SRC
    "chunk_d.cpp"
    "FindDialog.cpp"
    "mainfrm.cpp"
    "rawfilem.cpp"
    "stdafx.cpp"
    "wdeview.cpp"
    "wdlview.cpp"
    "wdtview.cpp"
    "wdump.cpp"
    "wdumpdoc.cpp"
    "wdview.cpp"
)

add_executable(z_wdump WIN32)
set_target_properties(z_wdump PROPERTIES OUTPUT_NAME wdump)

target_sources(z_wdump PRIVATE ${WDUMP_SRC})

target_link_libraries(z_wdump PRIVATE
    core_config
    core_wwstub # avoid linking GameEngine
    dbghelplib
    imm32
    vfw32
    winmm
    z_wwvegas
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(z_wdump PRIVATE _AFXDLL)
    target_sources(z_wdump PRIVATE wdump.rc)
endif()
