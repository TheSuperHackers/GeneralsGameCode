add_executable(z_imagepacker WIN32)
set_target_properties(z_imagepacker PROPERTIES OUTPUT_NAME imagepacker)

target_link_libraries(z_imagepacker PRIVATE
    corei_imagepacker
    z_debug
    z_gameengine
    z_gameenginedevice
    z_profile
    zi_always
)
