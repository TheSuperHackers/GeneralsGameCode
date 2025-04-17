add_executable(z_w3dview WIN32)

target_link_libraries(z_w3dview PRIVATE
    core_w3dview
    z_wwaudio
    z_wwvegas
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME W3DViewZH)
else()
    set_target_properties(z_w3dview PROPERTIES OUTPUT_NAME w3dviewzh)
endif()
