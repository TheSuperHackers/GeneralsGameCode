find_program(CLANG_FORMAT_EXECUTABLE
             NAMES clang-format-19
                   clang-format
             DOC "clang-format executable")
mark_as_advanced(CLANG_FORMAT_EXECUTABLE)

if(CLANG_FORMAT_EXECUTABLE)
    if(NOT CLANG_FORMAT_FOUND)
        execute_process(COMMAND ${CLANG_FORMAT_EXECUTABLE} -version
                        OUTPUT_VARIABLE clang_format_version
                        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

        if(clang_format_version MATCHES "^.*clang-format version .*")
            string(REGEX
                   REPLACE ".*clang-format version ([.0-9]+).*"
                           "\\1"
                           clang_format_version_parsed
                           "${clang_format_version}")

            set(CLANG_FORMAT_VERSION ${clang_format_version_parsed} CACHE INTERNAL "clang-format executable version")
            set(CLANG_FORMAT_FOUND TRUE CACHE INTERNAL "clang-format executable found")
        endif()

        unset(clang_format_version)
        unset(clang_format_version_parsed)
    endif()
else()
    set(CLANG_FORMAT_FOUND FALSE CACHE INTERNAL "clang-format executable found")
endif()
