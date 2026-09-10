add_executable(z_w3dview WIN32)
set_property(TARGET z_w3dview PROPERTY CXX_SCAN_FOR_MODULES ON)
add_dependencies(z_w3dview generals_graphics_shaders)
add_custom_command(TARGET z_w3dview POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/engine/graphics/shaders/$<CONFIG>"
        "$<TARGET_FILE_DIR:z_w3dview>/GraphicsShaders")

target_link_libraries(z_w3dview PRIVATE
    generals_graphics
    core_wwstub # avoid linking GameEngine
    corei_w3dview # this interface gets the source files for the tool
    d3d9lib
    imm32
    Version
    vfw32
    winmm
    z_wwaudio
    z_wwvegas
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(z_w3dview PRIVATE _AFXDLL)
    set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME "W3DViewZH${RTS_BUILD_OUTPUT_SUFFIX}")
else()
    set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME "w3dviewzh${RTS_BUILD_OUTPUT_SUFFIX}")
endif()
