add_subdirectory(DatGen)

add_executable(g_launcher WIN32)
set_target_properties(g_launcher PROPERTIES OUTPUT_NAME launcher)

target_link_libraries(g_launcher PRIVATE
    corei_launcher
    gi_always
)
