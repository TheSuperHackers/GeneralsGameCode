add_executable(z_datgen WIN32)
set_target_properties(z_datgen PROPERTIES OUTPUT_NAME datgen)

target_link_libraries(z_datgen PRIVATE
    corei_datgen
    zi_always
)
