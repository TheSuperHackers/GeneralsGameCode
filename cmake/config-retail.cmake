# Retail game compatibility options

set(RTS_BUILD_OPTION_RETAIL_COMPATIBLE_GAME "DEFAULT" CACHE STRING "Build for retail game compatibility. OFF enables retail-incompatible fixes. DEFAULT follows GameDefines.h.")
set_property(CACHE RTS_BUILD_OPTION_RETAIL_COMPATIBLE_GAME PROPERTY STRINGS DEFAULT ON OFF)

collect_defines_from_file(_retail_guards "${CMAKE_CURRENT_SOURCE_DIR}/Core/GameEngine/Include/Common/GameDefines.h" "RETAIL_COMPATIBLE_")
define_tristate_option(RTS_BUILD_OPTION_RETAIL_COMPATIBLE_GAME RetailCompatibleGame "Build with Retail Compatibility" "${_retail_guards}" "")
unset(_retail_guards)

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "12.0.8804" AND NOT RTS_BUILD_OPTION_RETAIL_COMPATIBLE_GAME STREQUAL "OFF")
    message(NOTICE "")
    message(NOTICE "  Retail compatibility: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} is not CRC-compatible with retail.")
    message(NOTICE "  Retail builds need the VC6 SP6 compiler (12.00.8804).")
    message(NOTICE "")
endif()
