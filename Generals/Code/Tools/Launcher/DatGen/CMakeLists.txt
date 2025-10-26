add_executable(g_datgen WIN32)
set_target_properties(g_datgen PROPERTIES OUTPUT_NAME datgen)

target_link_libraries(g_datgen PRIVATE
    corei_datgen
    gi_always
)
