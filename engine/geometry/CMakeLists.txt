cmake_minimum_required(VERSION 3.28)

add_library(generals_geometry STATIC)
target_sources(generals_geometry PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES QueryBounds.cppm)
target_compile_features(generals_geometry PUBLIC cxx_std_23)
target_link_libraries(generals_geometry PUBLIC generals_assets)

if(BUILD_TESTING)
    find_package(Boost REQUIRED)
    add_executable(generals_geometry_query_bounds_test)
    target_sources(generals_geometry_query_bounds_test PRIVATE
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES QueryBounds.test.cppm)
    target_include_directories(generals_geometry_query_bounds_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_geometry_query_bounds_test PRIVATE generals_geometry)
    add_test(NAME generals_geometry_query_bounds_test COMMAND generals_geometry_query_bounds_test)
endif()
