cmake_minimum_required(VERSION 4.4.3)

add_library(generalszh_order_rules STATIC)
target_sources(generalszh_order_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    inputs/attack_input.cppm systems/order_system.cppm)
target_compile_features(generalszh_order_rules PUBLIC cxx_std_23)
target_link_libraries(generalszh_order_rules PUBLIC
    engine_gameplay_rts_orders engine_gameplay_rts_ai_squads generalszh_combat_rules)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(systems/order_system.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_order_rules_tests systems/order_system.test)
    target_link_libraries(generalszh_order_rules_tests PRIVATE generalszh_game_session Boost::included_unit_test_framework)
    add_test(NAME generalszh_order_rules_tests COMMAND generalszh_order_rules_tests)
    set_source_files_properties(inputs/attack_input.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_attack_input_tests inputs/attack_input.test)
    target_link_libraries(generalszh_attack_input_tests PRIVATE generalszh_order_rules Boost::included_unit_test_framework)
    add_test(NAME generalszh_attack_input_tests COMMAND generalszh_attack_input_tests)
endif()
