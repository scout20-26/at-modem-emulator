cmake_minimum_required(VERSION 3.16)

project(at_modem
    VERSION 1.0.0
    DESCRIPTION "Simple AT modem emulator for Linux"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(at_modem_lib
    src/matcher.cpp
    src/config.cpp
    src/serial_port.cpp
    src/modem.cpp
)

target_include_directories(at_modem_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(at_modem_lib PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Werror
)

add_executable(atmodem src/main.cpp)
target_link_libraries(atmodem PRIVATE at_modem_lib)

enable_testing()

add_executable(test_matcher tests/test_matcher.cpp)
target_link_libraries(test_matcher PRIVATE at_modem_lib)
add_test(NAME matcher COMMAND test_matcher)

add_executable(test_config tests/test_config.cpp)
target_link_libraries(test_config PRIVATE at_modem_lib)
add_test(NAME config COMMAND test_config)
