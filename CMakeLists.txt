cmake_minimum_required(VERSION 3.16)
project(embedded_freertos_notes C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(embedded_utils
    src/dma_ping_pong.c
    src/frame_parser.c
    src/ring_buffer.c
)

target_include_directories(embedded_utils PUBLIC include)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(embedded_utils PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
endif()

enable_testing()

add_executable(unit_tests tests/test_main.c)
target_link_libraries(unit_tests PRIVATE embedded_utils)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(unit_tests PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
endif()

add_test(NAME embedded_utils_tests COMMAND unit_tests)

