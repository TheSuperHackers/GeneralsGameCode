add_executable(z_worldbuilder WIN32)

target_link_libraries(z_worldbuilder PRIVATE
    core_debug
    core_profile_legacy
    corei_worldbuilder
    z_gameengine
    z_gameenginedevice
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(z_worldbuilder PRIVATE _AFXDLL)
    set_target_properties(z_worldbuilder PROPERTIES OUTPUT_NAME "WorldBuilderZH${RTS_BUILD_OUTPUT_SUFFIX}")
else()
    set_target_properties(z_worldbuilder PROPERTIES OUTPUT_NAME "worldbuilderzh${RTS_BUILD_OUTPUT_SUFFIX}")
endif()
