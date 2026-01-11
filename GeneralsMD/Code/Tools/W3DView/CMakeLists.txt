add_executable(z_w3dview WIN32)

target_link_libraries(z_w3dview PRIVATE
    core_config
    core_wwstub # avoid linking GameEngine
    corei_w3dview # this interface gets the source files for the tool
    d3d8
    d3d8lib
    d3dx8
    imm32
    milesstub
    Version
    vfw32
    winmm
    z_wwaudio
    z_wwvegas
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(z_w3dview PRIVATE _AFXDLL)
    set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME W3DViewZH)
else()
    set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME w3dviewzh)
endif()
