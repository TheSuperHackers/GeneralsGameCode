add_library(z_wwdownload STATIC)
set_target_properties(z_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_link_libraries(z_wwdownload PRIVATE
    corei_wwdownload
    zi_always
    zi_gameengine_include
)
