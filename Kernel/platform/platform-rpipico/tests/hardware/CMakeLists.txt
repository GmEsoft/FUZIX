cmake_minimum_required(VERSION 3.13)
set(PICO_COPY_TO_RAM 1)
include(../../pico_sdk_import.cmake)
project(rpipico_abi_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(PLATFORM_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." CACHE PATH "Platform source under test")
get_filename_component(KERNEL_DIR "${PLATFORM_DIR}/../.." ABSOLUTE)
if(NOT EXISTS "${KERNEL_DIR}/include/sysinfoblk.h")
    message(FATAL_ERROR "Generate the kernel sysinfoblk.h first; see README.md")
endif()
pico_sdk_init()
add_executable(rpipico_abi_test abi.c abi.S syscall.c
    ${PLATFORM_DIR}/tricks.S ${PLATFORM_DIR}/main.c
    ${KERNEL_DIR}/cpu-armm0/lowlevel-armm0.c)
target_include_directories(rpipico_abi_test PRIVATE ${PLATFORM_DIR}
    ${KERNEL_DIR} ${KERNEL_DIR}/include ${KERNEL_DIR}/dev ${KERNEL_DIR}/cpu-armm0)
set_source_files_properties(${PLATFORM_DIR}/tricks.S PROPERTIES
    COMPILE_DEFINITIONS "__StackTop=abi_swap_stack_top")
set_source_files_properties(${PLATFORM_DIR}/main.c PROPERTIES
    COMPILE_DEFINITIONS "main=unused_platform_main;syscall_handler=abi_platform_syscall_handler")
set_source_files_properties(${KERNEL_DIR}/cpu-armm0/lowlevel-armm0.c PROPERTIES
    COMPILE_DEFINITIONS "deliver_signals=abi_platform_deliver_signals")
target_link_libraries(rpipico_abi_test pico_stdlib pico_multicore)
pico_enable_stdio_uart(rpipico_abi_test 0)
pico_enable_stdio_usb(rpipico_abi_test 1)
pico_add_extra_outputs(rpipico_abi_test)
