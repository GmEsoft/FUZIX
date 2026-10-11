cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(fuzix C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(PICO_COPY_TO_RAM 0)
set(TOTALMEM "160" CACHE STRING "User process memory pool in KiB")
set(PROCMEM "64" CACHE STRING "Maximum resident process size in KiB, including udata")
foreach(memory_setting TOTALMEM PROCMEM)
    if(NOT "${${memory_setting}}" MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "${memory_setting} must be a positive integer in KiB")
    endif()
endforeach()

add_compile_definitions(
		PICO_HEAP_SIZE=0x0
		PICO_NO_BINARY_INFO=1
		PICO_TIME_DEFAULT_ALARM_POOL_DISABLED
    TOTALMEM=${TOTALMEM}
    PROCMEM=${PROCMEM}
		)

pico_sdk_init()

# One geometry for the driver, linker checks and offline image tools. Board
# flash sizes may be SDK arithmetic expressions rather than decimal numbers.
set(FLASH_FIRMWARE_KIB "96" CACHE STRING "Flash reserved for firmware in KiB")
set(FLASH_XIP_KIB "0" CACHE STRING "Flash reserved for immutable user images in KiB")
foreach(flash_setting FLASH_FIRMWARE_KIB FLASH_XIP_KIB)
    if(NOT "${${flash_setting}}" MATCHES "^[0-9]+$")
        message(FATAL_ERROR "${flash_setting} must be a nonnegative integer in KiB")
    endif()
endforeach()
math(EXPR FUZIX_FLASH_BYTES "${PICO_FLASH_SIZE_BYTES}")
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS flash-layout.py)
execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/flash-layout.py
    --flash-bytes ${FUZIX_FLASH_BYTES} --firmware-kib ${FLASH_FIRMWARE_KIB}
    --xip-kib ${FLASH_XIP_KIB} --output ${CMAKE_CURRENT_BINARY_DIR}
    RESULT_VARIABLE FLASH_LAYOUT_RESULT)
if(NOT FLASH_LAYOUT_RESULT EQUAL 0)
    message(FATAL_ERROR "Invalid FUZIX flash layout")
endif()

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	.
	../..
	../../dev
	../../dev/net
	../../include
	../../cpu-armm0
)

add_executable(fuzix
	devices.c
	devflash.c
	devsdspi.c
	devtty.c
	elf.c 
	main.c
	misc.c
	rawflash.c
	rawuart.c
	swapper.c
	tricks.S
	core1.c
	devgpio.c
	usbdescriptors.c
	wiznet.c
	wiznet_spi.c
	../../dev/blkdev.c
	../../dev/mbr.c
	../../dev/devsd_discard.c
	../../dev/devsd.c
	../../lib/dhara/error.c
	../../lib/dhara/journal.c
	../../lib/dhara/map.c
	../../lib/hexdump.c
	../../blk512.c
	../../devio.c
	../../devsys.c
	../../filesys.c
	../../inode.c
	../../kdata.c
	../../cpu-armm0/lowlevel-armm0.c
	../../mm/malloc.c
	../../mm.c
	../../mm/memalloc_none.c
	../../process.c
	../../start.c
	../../swap.c
	../../syscall_execelf32.c
	../../syscall_exec.c
	../../syscall_fs.c
	../../syscall_fs2.c
	../../syscall_fs3.c
	../../syscall_other.c
	../../syscall_proc.c
	../../timer.c
	../../tty.c
	../../usermem.c
	../../version.c
	../../dev/net/net_native.c
	../../dev/net/net_w5x00.c
	../../syscall_net.c
	../../network.c
)

target_link_libraries(fuzix
	pico_stdlib
	pico_multicore
	pico_flash
	hardware_flash
	hardware_spi
	tinyusb_device
)

pico_set_float_implementation(fuzix none)
pico_set_double_implementation(fuzix none)
pico_add_extra_outputs(fuzix)
target_link_options(fuzix PRIVATE "LINKER:-T,${CMAKE_CURRENT_BINARY_DIR}/flash-layout.ld")
pico_add_link_depend(fuzix ${CMAKE_CURRENT_BINARY_DIR}/flash-layout.ld)
