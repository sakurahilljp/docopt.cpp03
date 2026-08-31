set(CMAKE_POLICY_VERSION_MINIMUM 3.14)
cmake_minimum_required(VERSION 3.14)
project(docopt_cpp03 CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wshadow -Wsign-compare)
endif()

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_definitions(-DDOCOPT_USE_CUSTOM_LEXICAL_CAST)

# docopt library
add_library(docopt STATIC docopt.cpp docopt.h)
add_library(docoptcpp03::docopt ALIAS docopt)
target_include_directories(docopt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
if(NOT MSVC)
    target_compile_options(docopt PRIVATE -std=c++03)
endif()

# Installation & Export targets
include(GNUInstallDirs)
install(TARGETS docopt
    EXPORT docoptcpp03Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES docopt.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT docoptcpp03Targets
    FILE docoptcpp03Targets.cmake
    NAMESPACE docoptcpp03::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/docoptcpp03
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/docoptcpp03Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/docoptcpp03Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/docoptcpp03"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/docoptcpp03Config.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/docoptcpp03"
)

# Legacy docopt testcases runner executable
add_executable(test_docopt test_docopt.cpp)
target_link_libraries(test_docopt PRIVATE docopt)
if(NOT MSVC)
    target_compile_options(test_docopt PRIVATE -std=c++03)
endif()

# Fetch GoogleTest
include(FetchContent)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
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

