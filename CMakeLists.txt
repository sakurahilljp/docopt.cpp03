set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
cmake_minimum_required(VERSION 3.5)
project(docopt_cpp03 CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-std=c++03 -Wall -Wextra)
endif()

add_definitions(-DDOCOPT_USE_CUSTOM_LEXICAL_CAST)

# docopt library
add_library(docopt STATIC docopt.cpp docopt.h)
target_include_directories(docopt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Legacy docopt testcases runner executable
add_executable(test_docopt test_docopt.cpp)
target_link_libraries(test_docopt PRIVATE docopt)

# Fetch GoogleTest (release-1.8.1 supports C++03)
include(FetchContent)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.8.1.zip
)
FetchContent_MakeAvailable(googletest)

# Comprehensive GoogleTest executable
add_executable(unit_tests unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE docopt gtest gtest_main)

enable_testing()
add_test(NAME docopt_testcases COMMAND test_docopt "${CMAKE_CURRENT_SOURCE_DIR}/docopt.python/testcases.docopt")
add_test(NAME docopt_gtest_unit_tests COMMAND unit_tests)

# Sample executables under cmds/
add_executable(naval_fate cmds/naval_fate/main.cpp)
target_link_libraries(naval_fate PRIVATE docopt)

add_executable(calculator cmds/calculator/main.cpp)
target_link_libraries(calculator PRIVATE docopt)
