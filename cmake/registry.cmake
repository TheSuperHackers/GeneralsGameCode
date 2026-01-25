# Helper macro for querying Windows registry values
# Queries both 32-bit and 64-bit registry views automatically
macro(fetch_registry_value registry_key registry_value output_var description)
    if(NOT ${output_var})
        cmake_host_system_information(RESULT _variable
            QUERY WINDOWS_REGISTRY
            "${registry_key}"
            VALUE "${registry_value}"
            VIEW 32_64)
        if(_variable)
            set(${output_var} "${_variable}" CACHE PATH "${description}")
        endif()
    endif()
endmacro()

