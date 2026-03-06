add_executable(z_mapcachebuilder WIN32)
set_target_properties(z_mapcachebuilder PROPERTIES OUTPUT_NAME "mapcachebuilder${RTS_BUILD_OUTPUT_SUFFIX}")

target_link_libraries(z_mapcachebuilder PRIVATE
    corei_mapcachebuilder
    z_gameengine
    z_gameenginedevice
    zi_always
)
