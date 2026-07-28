add_executable(g_worldbuilder WIN32)

target_link_libraries(g_worldbuilder PRIVATE
    corei_worldbuilder
    g_gameengine
    g_gameenginedevice
    gi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(g_worldbuilder PRIVATE _AFXDLL)
    set_target_properties(g_worldbuilder PROPERTIES OUTPUT_NAME "WorldBuilderV${RTS_BUILD_OUTPUT_SUFFIX}")
else()
    set_target_properties(g_worldbuilder PROPERTIES OUTPUT_NAME "worldbuilderv${RTS_BUILD_OUTPUT_SUFFIX}")
endif()
