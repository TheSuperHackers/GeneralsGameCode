add_executable(rts_crashpad_test CrashpadTest.cpp)
target_compile_features(rts_crashpad_test PRIVATE cxx_std_20)
target_compile_options(rts_crashpad_test PRIVATE /Zi)
target_link_options(rts_crashpad_test PRIVATE /DEBUG)
target_link_libraries(rts_crashpad_test PRIVATE rts_crashpad)
set_target_properties(rts_crashpad_test PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
add_custom_command(TARGET rts_crashpad_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:rts_crashpad>" "${handler}" "$<TARGET_FILE_DIR:rts_crashpad_test>"
    VERBATIM)

add_executable(rts_crashpad_seed SeedDatabase.cpp)
target_compile_features(rts_crashpad_seed PRIVATE cxx_std_20)
target_link_libraries(rts_crashpad_seed PRIVATE crashpad::crashpad)
set_target_properties(rts_crashpad_seed PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_executable(rts_crashpad_wrong_handler WrongHandler.cpp)
set_target_properties(rts_crashpad_wrong_handler PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
add_dependencies(rts_crashpad_test rts_crashpad_wrong_handler)
