add_subdirectory(Dependencies)
add_subdirectory(Core)

if(RTS_BUILD_ZEROHOUR)
    add_executable(z_googletest)
    target_link_libraries(z_googletest PRIVATE
        corei_googletest
        z_gameenginedevice
        zi_always
    )
    add_test(NAME z_googletest COMMAND z_googletest)
endif()

if(RTS_BUILD_GENERALS)
    add_executable(g_googletest)
    target_link_libraries(g_googletest PRIVATE
        corei_googletest
        g_gameenginedevice
        gi_always
    )
    add_test(NAME g_googletest COMMAND g_googletest)
endif()
