# Custom clang-tidy plugin for GeneralsGameCode
# This plugin provides checks for custom types like AsciiString and UnicodeString

cmake_minimum_required(VERSION 3.20)
project(GeneralsGameCodeClangTidyPlugin)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

# Set up include directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Add definitions
add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
  GeneralsGameCodeTidyModule.cpp
  readability/UseIsEmptyCheck.cpp
  readability/UseThisInsteadOfSingletonCheck.cpp
)

# Header files
set(HEADERS
  GeneralsGameCodeTidyModule.h
  readability/UseIsEmptyCheck.h
  readability/UseThisInsteadOfSingletonCheck.h
)

# Create the module library
add_library(GeneralsGameCodeClangTidyPlugin MODULE ${SOURCES} ${HEADERS})

# Link against required libraries
target_link_libraries(GeneralsGameCodeClangTidyPlugin
  PRIVATE
  clangTidy
  clangTidyReadabilityModule
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangLex
  clangTooling
  LLVMSupport
)

# Set output directory
set_target_properties(GeneralsGameCodeClangTidyPlugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

