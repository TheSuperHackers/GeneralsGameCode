add_subdirectory(DatGen)

add_executable(z_launcher WIN32)
set_target_properties(z_launcher PROPERTIES OUTPUT_NAME launcher)

target_link_libraries(z_launcher PRIVATE
    corei_launcher
    zi_always
)
