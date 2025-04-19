add_executable(g_w3dview WIN32)

target_link_libraries(g_w3dview PRIVATE
    core_w3dview
    g_wwaudio
    g_wwvegas
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(g_w3dview PROPERTIES OUTPUT_NAME W3DViewV)
else()
    set_target_properties(g_w3dview PROPERTIES OUTPUT_NAME w3dviewzv)
endif()
