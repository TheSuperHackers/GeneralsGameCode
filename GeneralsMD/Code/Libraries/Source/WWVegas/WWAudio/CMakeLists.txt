add_library(z_wwaudio STATIC)
set_target_properties(z_wwaudio PROPERTIES OUTPUT_NAME wwaudio)

target_link_libraries(z_wwaudio PRIVATE
    corei_wwaudio
    z_wwcommon
)
