# TheSuperHackers: Target exports module for cross-platform symbol export
# Test for linker support for --version-script
if(NOT MSVC)
    include(CheckCXXSourceCompiles)
    # Check if LD supports linker scripts.
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/conftest.map" "VERS_1 {
        global: sym;
    };

    VERS_2 {
        global: sym;
    } VERS_1;"
    )
    set(CMAKE_REQUIRED_FLAGS_SAVE ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} "-Wl,--version-script=${CMAKE_CURRENT_BINARY_DIR}/conftest.map")
    check_cxx_source_compiles("int main(int argc, char **argv){return 0;}" HAVE_LD_VERSION_SCRIPT)
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_SAVE})
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/conftest.map")
endif()

function(target_exports target)
    cmake_parse_arguments(SCRIPTS "" "" "SYMBOLS" ${ARGN})
    if(NOT SCRIPTS_SYMBOLS)
        message(ERROR "No symbols provided for export.")
    elseif(MSVC)
        # Create a def file for msvc.
        set(_symbols "EXPORTS\n${SCRIPTS_SYMBOLS}")
        string(REPLACE ";" "\n" _symbols "${_symbols}")
        set(_def_file "${CMAKE_CURRENT_BINARY_DIR}/${target}symbols.def")
        file(WRITE ${_def_file} "${_symbols}\n")
        target_sources(${target} PRIVATE ${_def_file})
    elseif(HAVE_LD_VERSION_SCRIPT)
        # Create a version script for newer toolchains.
        set(_symbols "{ global: ${SCRIPTS_SYMBOLS}; local: *; };")
        set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/${target}symbols.map")
        file(WRITE ${_version_script} "${_symbols}\n")
        target_link_options(${target} PRIVATE
            "-Wl,--version-script=${_version_script}"
        )
    elseif(APPLE)
        # Fallback for old toolchains, create a symbols_list file for the Darwin linker.
        string(REPLACE ";" "\n" _symbols "${SCRIPTS_SYMBOLS}")
        set(_symbols_list "${CMAKE_CURRENT_BINARY_DIR}/${target}symbols.list")
        file(WRITE ${_symbols_list} "_${_symbols}\n")
        target_link_options(${target} PRIVATE
            "-exported_symbols_list,'${_symbols_list}'"
        )
    else()
        message(WARN "No suitable linker options for your tool chain to export defined symbols.")
    endif()
endfunction()