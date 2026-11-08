cmake_minimum_required(VERSION 3.10)
project(ch32v307_demo C)

set(CMAKE_C_STANDARD 99)
set(SYN_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_executable(ch32v307_demo
    main.c
    board_evt.c
    ${SYN_ROOT}/src/port/ch32v307/port_ch32v307.c
    ${SYN_ROOT}/src/port/ch32v307/startup_ch32v307.c
    ${SYN_ROOT}/src/syntropic/sched/syn_sched.c
    ${SYN_ROOT}/src/syntropic/sched/syn_mutex.c
    ${SYN_ROOT}/src/syntropic/sched/syn_timer.c
    ${SYN_ROOT}/src/syntropic/sched/syn_event_flags.c
    ${SYN_ROOT}/src/syntropic/port_stubs/syn_port_stubs.c
)

target_compile_definitions(ch32v307_demo PRIVATE CH32V307xx=1)
target_compile_options(ch32v307_demo PRIVATE -ffunction-sections -fdata-sections)
target_include_directories(ch32v307_demo PRIVATE
    ${SYN_ROOT}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_options(ch32v307_demo PRIVATE -Wl,--gc-sections)
