# Helper macro for querying Windows registry paths
# Queries both 32-bit and 64-bit registry views automatically
macro(try_set_install_prefix_from_registry output_var registry_key registry_value description)
    if(NOT ${output_var})
        cmake_host_system_information(RESULT _temp_path
            QUERY WINDOWS_REGISTRY
            "${registry_key}"
            VALUE ${registry_value}
            VIEW 32_64)
        if(_temp_path)
            set(${output_var} "${_temp_path}" CACHE PATH "${description}")
        endif()
    endif()
endmacro()

