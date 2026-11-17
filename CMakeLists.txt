set(CMAKE_POLICY_VERSION_MINIMUM 3.11)
cmake_minimum_required(VERSION 3.11)
project(docopt_cpp03 CXX)

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
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address,undefined")
endif()

# docopt library
add_library(docopt STATIC docopt.cpp docopt.h)
add_library(docoptcpp03::docopt ALIAS docopt)
target_include_directories(docopt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

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

# Extract and build vendored GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

set(GTEST_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest-release-1.7.0.tar.gz")
set(GTEST_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/googletest-release-1.7.0")
if(NOT EXISTS "${GTEST_SRC_DIR}/CMakeLists.txt")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar xzf "${GTEST_ARCHIVE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps"
        RESULT_VARIABLE GTEST_EXTRACT_RESULT
    )
    if(NOT GTEST_EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract ${GTEST_ARCHIVE}")
    endif()
endif()
add_subdirectory("${GTEST_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/_deps/googletest-build" EXCLUDE_FROM_ALL)

# Comprehensive GoogleTest executable
add_executable(unit_tests unit_tests.cpp)
target_include_directories(unit_tests SYSTEM PRIVATE "${GTEST_SRC_DIR}/include")
target_link_libraries(unit_tests PRIVATE docopt gtest gtest_main)

enable_testing()
add_test(NAME docopt_testcases COMMAND test_docopt "${CMAKE_CURRENT_SOURCE_DIR}/docopt.python/testcases.docopt")
add_test(NAME docopt_gtest_unit_tests COMMAND unit_tests)

# Sample executables under cmds/
add_executable(naval_fate cmds/naval_fate/main.cpp)
target_link_libraries(naval_fate PRIVATE docopt)

add_executable(calculator cmds/calculator/main.cpp)
target_link_libraries(calculator PRIVATE docopt)

# Enforce strict C++03 conformance across all project targets (excluding third-party GoogleTest)
if(NOT MSVC)
    foreach(target IN ITEMS docopt test_docopt unit_tests naval_fate calculator)
        target_compile_options(${target} PRIVATE -std=c++03 -pedantic-errors)
    endforeach()
endif()

