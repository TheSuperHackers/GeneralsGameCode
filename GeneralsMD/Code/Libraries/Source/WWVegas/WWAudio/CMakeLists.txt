add_library(z_wwaudio STATIC)
set_target_properties(z_wwaudio PROPERTIES OUTPUT_NAME wwaudio)

# Public audio headers expose Miles types.
target_link_libraries(z_wwaudio PUBLIC milesstub)

target_link_libraries(z_wwaudio PRIVATE
    corei_wwaudio
    z_wwcommon
    zi_always
)
