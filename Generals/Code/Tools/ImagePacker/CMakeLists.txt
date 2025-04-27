add_executable(g_imagepacker WIN32)
set_target_properties(g_imagepacker PROPERTIES OUTPUT_NAME imagepacker)

target_link_libraries(g_imagepacker PRIVATE
    corei_imagepacker
    g_gameengine
    g_gameenginedevice
    gi_always
)
