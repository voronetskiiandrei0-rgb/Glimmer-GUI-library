cmake_minimum_required(VERSION 3.16)
project(Glimmer LANGUAGES CXX)

include(CTest)
include(FetchContent)

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

add_library(glimmer INTERFACE)
target_include_directories(glimmer INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(glimmer INTERFACE cxx_std_17)



# Your personal application
add_executable(my_app test.cpp)
target_link_libraries(my_app PRIVATE glimmer)

add_executable(settings_panel examples/settings_panel.cpp)
target_link_libraries(settings_panel PRIVATE glimmer)

if(WIN32)
    add_executable(glimmer_live_demo WIN32 examples/live_demo_win32.cpp)
    target_link_libraries(glimmer_live_demo PRIVATE glimmer user32 gdi32)
endif()

if(BUILD_TESTING)
    FetchContent_Declare(
        doctest
        URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_GetProperties(doctest)
    if(NOT doctest_POPULATED)
        FetchContent_Populate(doctest)
    endif()
    add_library(glimmer_doctest INTERFACE)
    add_library(doctest::doctest ALIAS glimmer_doctest)
    target_include_directories(glimmer_doctest SYSTEM INTERFACE
        "${doctest_SOURCE_DIR}"
    )

    add_executable(glimmer_tests tests/test_glimmer.cpp)
    target_link_libraries(glimmer_tests PRIVATE glimmer doctest::doctest)
    add_test(NAME glimmer_tests COMMAND glimmer_tests)
endif()

if(MSVC)
    target_compile_options(settings_panel PRIVATE /W4 /WX)
    if(TARGET glimmer_live_demo)
        target_compile_options(glimmer_live_demo PRIVATE /W4 /WX)
    endif()
    if(TARGET glimmer_tests)
        target_compile_options(glimmer_tests PRIVATE /W4 /WX)
    endif()
else()
    target_compile_options(settings_panel PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(TARGET glimmer_live_demo)
        target_compile_options(glimmer_live_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    if(TARGET glimmer_tests)
        target_compile_options(glimmer_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()
