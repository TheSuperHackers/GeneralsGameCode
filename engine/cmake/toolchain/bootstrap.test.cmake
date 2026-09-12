# Negative cases must fail before downloading or extracting anything.
foreach(case unknown-host relative-directory)
    if(case STREQUAL "unknown-host")
        set(host unsupported)
        set(destination "${CMAKE_CURRENT_BINARY_DIR}/bootstrap-negative-test")
        set(expected "No pinned binary package")
    else()
        set(host linux-x64)
        set(destination relative-directory)
        set(expected "Supply -DENGINE_HOST")
    endif()
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DENGINE_HOST=${host}"
        "-DENGINE_TOOLS_DIR=${destination}" -P "${CMAKE_CURRENT_LIST_DIR}/bootstrap.cmake"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT error MATCHES "${expected}")
        message(FATAL_ERROR "Bootstrap failed to reject ${case}: ${output}${error}")
    endif()
endforeach()
