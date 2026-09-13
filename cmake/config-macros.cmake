# Shared helper macros for the config-*.cmake files.

# Toggles a DEFAULT/ON/OFF option over DefineNames (a name or list of names). ON -> <name>=1.
# OffDefineNames empty -> OFF sets the same names to <name>=0.
# OffDefineNames non-empty -> OFF sets these names to <off_name>=1 instead, one per DefineNames entry.
macro(define_tristate_option OptionName FeatureInfoName FeatureInfoDescription DefineNames OffDefineNames)
    set(_tristate_names "${DefineNames}")
    if(NOT _tristate_names)
        message(FATAL_ERROR "DefineNames must be a non-empty name or list of names.")
    endif()

    set(_tristate_off_names "${OffDefineNames}")

    set(_tristate_on_defs "")
    set(_tristate_off_defs "")
    foreach(_tristate_name _tristate_off_name IN ZIP_LISTS _tristate_names _tristate_off_names)
        if(_tristate_off_names AND (NOT _tristate_name OR NOT _tristate_off_name))
            message(FATAL_ERROR "OffDefineNames must be empty or match DefineNames in length.")
        endif()
        list(APPEND _tristate_on_defs "${_tristate_name}=1")
        if(_tristate_off_names)
            list(APPEND _tristate_off_defs "${_tristate_off_name}=1")
        else()
            list(APPEND _tristate_off_defs "${_tristate_name}=0")
        endif()
    endforeach()

    if(${OptionName} STREQUAL "DEFAULT")
        # Does nothing
    elseif(${OptionName} STREQUAL "ON")
        target_compile_definitions(core_config INTERFACE ${_tristate_on_defs})
        add_feature_info(${FeatureInfoName} TRUE ${FeatureInfoDescription})
    elseif(${OptionName} STREQUAL "OFF")
        target_compile_definitions(core_config INTERFACE ${_tristate_off_defs})
        add_feature_info(${FeatureInfoName} FALSE ${FeatureInfoDescription})
    else()
        message(FATAL_ERROR "Unhandled ${OptionName} value: ${${OptionName}}")
    endif()

    unset(_tristate_names)
    unset(_tristate_off_names)
    unset(_tristate_on_defs)
    unset(_tristate_off_defs)
    unset(_tristate_name)
    unset(_tristate_off_name)
endmacro()

# Collects defines matching DefinePrefix from InputFile into OutputVariableName.
macro(collect_defines_from_file OutputVariableName InputFile DefinePrefix)
    file(STRINGS "${InputFile}" _matching_lines REGEX "^#[ \t]*define[ \t]+${DefinePrefix}[A-Z0-9_]+")
    set(${OutputVariableName} "")
    foreach(_matching_line IN LISTS _matching_lines)
        string(REGEX MATCH "${DefinePrefix}[A-Z0-9_]+" _define_name "${_matching_line}")
        list(APPEND ${OutputVariableName} "${_define_name}")
    endforeach()
    list(REMOVE_DUPLICATES ${OutputVariableName})

    if(NOT ${OutputVariableName})
        message(FATAL_ERROR "No ${DefinePrefix}* defines found in ${InputFile}.")
    endif()

    unset(_matching_lines)
    unset(_matching_line)
    unset(_define_name)
endmacro()
