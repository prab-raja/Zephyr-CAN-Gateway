cmake_minimum_required(VERSION 3.20.0)

# Compiler flag configurations
set(CONFIG_CAN_CONTROLLER "1" CACHE STRING "CAN Controller selection (1 or 2)")
set(CONFIG_USE_CANFD "n" CACHE STRING "Enable CAN-FD support (y or n)")
set(CONFIG_FILTER_MODE "whitelist" CACHE STRING "Filter mode (whitelist or blacklist)")

# Pass to C preprocessor
add_compile_definitions(
    CAN_CONTROLLER=${CONFIG_CAN_CONTROLLER}
    USE_CANFD=$<IF:$<STREQUAL:${CONFIG_USE_CANFD},y>,1,0>
    FILTER_MODE_WHITELIST=$<IF:$<STREQUAL:${CONFIG_FILTER_MODE},whitelist>,1,0>
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(can_gateway)

# Source files
target_sources(app PRIVATE
    src/main.c
    src/can_router.c
    src/uart_diagnostics.c
    src/filter.c
)

# Include directories
target_include_directories(app PRIVATE
    src
)

# Print configuration
message(STATUS "========================================")
message(STATUS "CAN Gateway Configuration:")
message(STATUS "  CAN Controller: ${CONFIG_CAN_CONTROLLER}")
message(STATUS "  CAN-FD Support: ${CONFIG_USE_CANFD}")
message(STATUS "  Filter Mode: ${CONFIG_FILTER_MODE}")
message(STATUS "========================================")