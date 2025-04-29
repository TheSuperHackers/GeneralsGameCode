add_library(g_wwdownload STATIC)
set_target_properties(g_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_link_libraries(g_wwdownload PRIVATE
    corei_wwdownload
    gi_always
    gi_gameengine_include
)
