cmake_minimum_required(VERSION 3.20)

project(
    yadro_tape_sort
    VERSION 1.0.0
    DESCRIPTION "Tape storage emulator and external sorter"
    LANGUAGES CXX
)

option(YADRO_BUILD_TESTS "Build unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_library(yadro_tape_core STATIC
    src/FileTape.cpp
    src/TapeConfig.cpp
    src/TapeSorter.cpp
)

target_include_directories(yadro_tape_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_compile_features(yadro_tape_core
    PUBLIC
        cxx_std_20
)

add_executable(yadro_tape_sort
    src/main.cpp
)

target_link_libraries(yadro_tape_sort
    PRIVATE
        yadro_tape_core
)

add_executable(tape_tool
    src/tape_tool.cpp
)

foreach(target_name yadro_tape_core yadro_tape_sort tape_tool)
    if(MSVC)
        target_compile_options(${target_name}
            PRIVATE
                /W4
                /permissive-
                /utf-8
        )
    else()
        target_compile_options(${target_name}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
        )
    endif()
endforeach()

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/tmp")
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tmp")

if(EXISTS "${PROJECT_SOURCE_DIR}/config.txt")
    configure_file(
        "${PROJECT_SOURCE_DIR}/config.txt"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.txt"
        COPYONLY
    )
endif()

include(CTest)

if(YADRO_BUILD_TESTS AND EXISTS "${PROJECT_SOURCE_DIR}/tests/TapeSorterTests.cpp")
    include(FetchContent)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    FetchContent_MakeAvailable(googletest)

    add_executable(yadro_tape_tests
        tests/TapeSorterTests.cpp
    )

    target_link_libraries(yadro_tape_tests
        PRIVATE
            yadro_tape_core
            GTest::gtest_main
    )

    if(MSVC)
        target_compile_options(yadro_tape_tests
            PRIVATE
                /W4
                /permissive-
                /utf-8
        )
    else()
        target_compile_options(yadro_tape_tests
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
        )
    endif()

    include(GoogleTest)
    gtest_discover_tests(yadro_tape_tests)
endif()