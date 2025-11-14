add_executable(g_mapcachebuilder WIN32)
set_target_properties(g_mapcachebuilder PROPERTIES OUTPUT_NAME mapcachebuilder)

target_link_libraries(g_mapcachebuilder PRIVATE
    corei_mapcachebuilder
    g_gameengine
    g_gameenginedevice
    gi_always
)
