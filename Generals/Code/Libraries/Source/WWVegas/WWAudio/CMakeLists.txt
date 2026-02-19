add_library(g_wwaudio STATIC)
set_target_properties(g_wwaudio PROPERTIES OUTPUT_NAME wwaudio)

target_precompile_headers(g_wwaudio PRIVATE
    <windows.h>
)

target_link_libraries(g_wwaudio PRIVATE
    corei_wwaudio
    g_wwcommon
    gi_always
)
