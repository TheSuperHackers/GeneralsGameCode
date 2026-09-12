cmake_minimum_required(VERSION 3.25)
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "Pass an isolated -DTEST_ROOT=<temporary directory>")
endif()
# Isolated fixtures per invocation: changing a probe's source path must not leave
# yesterday's file in today's source tree. This identity is test IO, not simulation.
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef fixture_run)
set(TEST_ROOT "${TEST_ROOT}/${fixture_run}")
function(probe name path source expected)
    set(root "${TEST_ROOT}/${name}")
    get_filename_component(directory "${root}/${path}" DIRECTORY)
    file(MAKE_DIRECTORY "${directory}")
    file(WRITE "${root}/${path}" "${source}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DSOURCE_ROOT=${root}"
        -P "${CMAKE_CURRENT_LIST_DIR}/gameplay.cmake"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(expected STREQUAL "pass")
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "${name}: expected pass: ${output}${error}")
        endif()
    elseif(result EQUAL 0 OR NOT error MATCHES "${expected}")
        message(FATAL_ERROR "${name}: expected ${expected}: ${output}${error}")
    endif()
endfunction()
set(feature "engine/gameplay/example/example.cppm")
probe(doormixed "games/generalszh/gameplay/production/doors/production_door.cppm"
    "template<> struct ComponentTraits<DoorOpening> {};" component-outside-components)
probe(doorcomponent "games/generalszh/gameplay/production/doors/components/door.cppm"
    "template<> struct ComponentTraits<DoorOpening> {};" pass)
probe(doorwrongexecution "games/generalszh/gameplay/production/doors/components/door.cppm"
    "struct FactoryExitSystem {};" system-outside-systems)
probe(doorfacade "games/generalszh/gameplay/production/doors/production_door.cppm"
    "export import games.generalszh.gameplay.production.doors.components.door;" pass)
probe(componentanywhere "engine/gameplay/example/mixed.cppm" "template<> struct ComponentTraits<Value> {};" component-outside-components)
probe(systemanywhere "games/generalszh/gameplay/example/mixed.cppm" "struct RealSystem {};" system-outside-systems)
set(feature "engine/gameplay/example/systems/example.cppm")
probe(system "${feature}" "struct IncomeSystem { using Query = QueryType; void Execute(Query::Chunk, SystemContext&); };" pass)
probe(wrapper "${feature}" "class HarvestSystems {};" feature-wrapper)
probe(movementstep "${feature}" "struct MovementPrepareSystem {};" rejected-step-system)
probe(accountstep "${feature}" "struct AccountPublishSystem {};" rejected-step-system)
probe(snapshotstep "${feature}" "template<bool AfterMovement> class TargetSnapshotSystem {};" rejected-step-system)
probe(derivedindex "${feature}" "class TargetIndex { Query targets; void Rebuild(); };" pass)
probe(cohesivelifecycle "${feature}" "struct MovementSystem { using Query = QueryType; void BeforeChunks(Query&, SystemContext&); void Execute(Query::Chunk, SystemContext&); };" pass)
probe(adapterwrapper "games/generalszh/adapters/content/example.cppm" "class EconomySystems {};" feature-wrapper)
probe(hostwrapper "games/generalszh/hosts/example.cppm" "class FeatureSimulation {};" feature-wrapper)
probe(renamed "${feature}" "class HarvestFacade { ecs::Scheduler scheduler; };" feature-execution-owner-reference)
probe(alias "${feature}" "using Runner = ecs::Scheduler; class Facade { Runner loop; };" feature-execution-owner-reference)
probe(pointer "${feature}" "class Facade { std::unique_ptr<ecs::SystemRegistry> registry; };" feature-execution-owner-reference)
probe(registration "${feature}" "inline void RegisterHealthSystems(ecs::SystemRegistry &registry, HealthSystem &system) { registry.Register(system); }" pass)
probe(managerregistration "${feature}" "class Facade { void Register(ecs::SystemRegistry &registry); };" feature-execution-owner-reference)
probe(schedulerhelper "${feature}" "inline void RegisterBad(ecs::Scheduler &scheduler) {}" feature-execution-owner-reference)
probe(import "engine/events/example.cppm" "export import games.generalszh.rules;" engine-game-import)
probe(order "games/generalszh/simulation/orders/order_simulation.cppm"
    "class OrderInputSystem { HarvestState state; };" order-input-feature-coupling)
probe(root "games/generalszh/composition/game_session.cppm"
    "class GameSession { void Finalize() { } ecs::Scheduler scheduler; ecs::SystemRegistry registry; };" pass)
probe(legitImpl "games/generalszh/composition/game_session_impl.cppm"
    "class GameSession::Impl { ecs::Scheduler scheduler; }; const ecs::SystemRegistry &GameSession::Systems() const noexcept;" pass)
probe(implExtraOwner "games/generalszh/composition/game_session_impl.cppm"
    "class GameSession::Impl { ecs::Scheduler scheduler; }; class ExtraSystems {};" feature-wrapper)
probe(implExtraScheduler "games/generalszh/composition/game_session_impl.cppm"
    "class GameSession::Impl { ecs::Scheduler scheduler; }; class Facade { ecs::Scheduler scheduler; };" feature-execution-owner-reference)
probe(wrongPathImpl "games/generalszh/composition/other.cppm"
    "class GameSession::Impl { ecs::Scheduler scheduler; };" feature-execution-owner-reference)
probe(wrongAccessor "games/generalszh/composition/other.cppm"
    "const ecs::SystemRegistry &GameSession::Systems() const noexcept;" feature-execution-owner-reference)
probe(wrongroot "games/generalszh/composition/other.cppm"
    "class GameSession { ecs::Scheduler scheduler; };" feature-execution-owner-reference)
foreach(pair "income/IncomeSimulation" "lifetime/LifetimeSimulation" "power/PowerSimulation" "radar/RadarSimulation")
    string(REPLACE "/" ";" parts "${pair}")
    list(GET parts 0 feature_name)
    list(GET parts 1 type)
    set(path "games/generalszh/simulation/${feature_name}/${feature_name}_simulation.cppm")
    probe(${feature_name} "${path}" "class ${type} { ecs::Scheduler scheduler; };" pass)
endforeach()
probe(extra "games/generalszh/simulation/income/income_simulation.cppm"
    "class IncomeSimulation { ecs::Scheduler scheduler; }; class NewSystems {};" feature-wrapper)
probe(extraowner "games/generalszh/simulation/income/income_simulation.cppm"
    "class IncomeSimulation { ecs::Scheduler scheduler; }; class Facade { ecs::Scheduler scheduler; };" feature-execution-owner-reference)
probe(wrongtype "games/generalszh/simulation/income/income_simulation.cppm"
    "class OtherSimulation {};" feature-wrapper)
probe(comments "${feature}" "// class OldSystems {};\n/* ecs::Scheduler x; */\nstruct RealSystem {};" pass)
message(STATUS "Gameplay architecture checker self-tests passed")
