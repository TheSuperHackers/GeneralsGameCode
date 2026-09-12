# Small source-policy guard, not a C++ parser. Scope: production .cppm modules.
# Macros, raw strings, escaped-newline tokens and indirect/generated type names
# are not resolved. Semantic review of ownership and responsibility is mandatory.
cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED SOURCE_ROOT)
    get_filename_component(SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()

# Remove only the named declaration body, never its entire file. Brace matching
# here supports the ordinary declarations used by the five authorized owners.
function(remove_owner text type output)
    if(text MATCHES "(class|struct)[ \t\r\n]+${type}[ \t\r\n]*(final[ \t\r\n]*)?\\{")
        set(start "${CMAKE_MATCH_0}")
        string(FIND "${text}" "${start}" begin)
        string(LENGTH "${start}" length)
        math(EXPR position "${begin}+${length}")
        set(depth 1)
        string(LENGTH "${text}" total)
        while(depth GREATER 0 AND position LESS total)
            string(SUBSTRING "${text}" ${position} 1 character)
            if(character STREQUAL "{")
                math(EXPR depth "${depth}+1")
            elseif(character STREQUAL "}")
                math(EXPR depth "${depth}-1")
            endif()
            math(EXPR position "${position}+1")
        endwhile()
        if(NOT depth EQUAL 0)
            message(FATAL_ERROR "architecture: unsupported/unbalanced owner declaration ${type}")
        endif()
        string(SUBSTRING "${text}" 0 ${begin} prefix)
        string(SUBSTRING "${text}" ${position} -1 suffix)
        set(text "${prefix}${suffix}")
    endif()
    set(${output} "${text}" PARENT_SCOPE)
endfunction()

# Exact retained-native path/type pairs. No modern feature debt exceptions.
set(native_owners
    "income/IncomeSimulation" "lifetime/LifetimeSimulation"
    "power/PowerSimulation" "radar/RadarSimulation")
file(GLOB_RECURSE sources RELATIVE "${SOURCE_ROOT}"
    "${SOURCE_ROOT}/engine/*.cppm"
    "${SOURCE_ROOT}/games/generalszh/*.cppm")
list(SORT sources)
set(failures "")
include("${CMAKE_CURRENT_LIST_DIR}/layout_debt.cmake")
foreach(path IN LISTS sources)
    file(READ "${SOURCE_ROOT}/${path}" text)
    string(REGEX REPLACE "/\\*([^*]|\\*+[^*/])*\\*+/" " " text "${text}")
    string(REGEX REPLACE "//[^\n]*" " " text "${text}")
    if(path MATCHES "^engine/" AND text MATCHES "(^|[;\n])[ \t]*(export[ \t]+)?import[ \t]+(games[.]|generalszh[.])")
        string(APPEND failures "${path}: engine-game-import\n")
    endif()
    if(NOT path MATCHES "^(engine/gameplay/|games/generalszh/)")
        continue()
    endif()
    # Tree-wide protection; the exact existing debt is visible and must shrink.
    # This is a source-layout check, not proof of connected gameplay execution.
    if(path MATCHES "^(engine/gameplay/|games/generalszh/gameplay/)")
        if(text MATCHES "ComponentTraits[ \t\r\n]*<" AND NOT path MATCHES "/components/")
            if(NOT path IN_LIST gameplay_component_layout_debt)
                string(APPEND failures "${path}: component-outside-components\n")
            endif()
        endif()
        if(text MATCHES "(class|struct)[ \t\r\n]+[A-Za-z_][A-Za-z_0-9]*System([^A-Za-z_0-9]|$)" AND NOT path MATCHES "/systems/")
            if(NOT path IN_LIST gameplay_system_layout_debt)
                string(APPEND failures "${path}: system-outside-systems\n")
            endif()
        endif()
    endif()
    foreach(owner IN LISTS native_owners)
        string(REPLACE "/" ";" pair "${owner}")
        list(GET pair 0 feature)
        list(GET pair 1 type)
        if(path STREQUAL "games/generalszh/simulation/${feature}/${feature}_simulation.cppm")
            remove_owner("${text}" "${type}" text)
        endif()
    endforeach()
    if(path STREQUAL "games/generalszh/composition/game_session.cppm")
        remove_owner("${text}" "GameSession" text)
    endif()
    if(path STREQUAL "games/generalszh/composition/game_session_impl.cppm")
        # The composition implementation is the private body of the exported
        # GameSession root. Remove only that exact qualified declaration body;
        # declarations elsewhere in this file remain subject to every guard.
        remove_owner("${text}" "GameSession::Impl" text)
    endif()
    if(text MATCHES "(class|struct)[ \t\r\n]+[A-Za-z_][A-Za-z_0-9]*(Simulation|Systems)([^A-Za-z_0-9]|$)")
        string(APPEND failures "${path}: feature-wrapper (${CMAKE_MATCH_0})\n")
    endif()
    # Concrete regressions rejected during this cleanup. This is not a blanket
    # naming ban: method-sized behaviour under other names still needs review.
    if(text MATCHES "(class|struct)[ \t\r\n]+(AccountPrepareSystem|AccountPublishSystem|MovementPrepareSystem|PowerBuildGateSystem|OrderObservationSystem|OrderWeaponSystem|TargetSnapshotSystem)([^A-Za-z_0-9]|$)")
        string(APPEND failures "${path}: rejected-step-system (${CMAKE_MATCH_0})\n")
    endif()
    # Ordinary free startup helpers may receive the root-owned registry/systems.
    # Recognize only the existing unindented inline void Register... signature.
    # This is a syntax convention, not proof that a function is at namespace scope.
    set(ownership_text "${text}")
    string(REGEX MATCHALL "(^|\n)inline void Register[A-Za-z_0-9]*\\([^)]*\\)" signatures "${text}")
    foreach(signature IN LISTS signatures)
        string(REGEX REPLACE "(ecs::)?SystemRegistry[ \t\r\n]*&" "RootRegistryReference " permitted "${signature}")
        string(REPLACE "${signature}" "${permitted}" ownership_text "${ownership_text}")
    endforeach()
    if(path STREQUAL "games/generalszh/composition/game_session_impl.cppm")
        # The public accessor exposes the root-owned graph by reference. This
        # is the sole implementation-unit signature exempted from the owner
        # token scan; its body is intentionally still scanned.
        string(REGEX REPLACE "const[ \t]+ecs::SystemRegistry[ \t]*&GameSession::Systems\\(\\)[ \t]+const[ \t]+noexcept"
            "RootSystemRegistryAccessor" ownership_text "${ownership_text}")
    endif()
    # Conservative: references/aliases/unique_ptr as well as values. Dotted
    # lowercase module imports do not match these type names.
    if(ownership_text MATCHES "(^|[^A-Za-z_0-9])(Scheduler|SystemRegistry)([^A-Za-z_0-9]|$)")
        string(APPEND failures "${path}: feature-execution-owner-reference\n")
    endif()
    if(text MATCHES "Order(Input)?System" AND text MATCHES "harvesting|construction|HarvestState|HarvestPhase|BuilderAssignment|SetTaskActors")
        string(APPEND failures "${path}: order-input-feature-coupling\n")
    endif()
endforeach()
if(failures)
    message(FATAL_ERROR "Gameplay architecture violations:\n${failures}")
endif()
message(STATUS "Gameplay architecture source checks passed (regex scope; semantic review required)")
