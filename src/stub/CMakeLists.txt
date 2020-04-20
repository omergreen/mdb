cmake_minimum_required(VERSION 3.16.5)
set(CMAKE_SKIP_INSTALL_RULES true)
project(stub C ASM)

if(NOT DEFINED ARCH)
    message(FATAL_ERROR "ARCH is not defined, set it to one of arch/ subdirectories")
elseif(NOT EXISTS "${PROJECT_SOURCE_DIR}/arch/${ARCH}")
    message(FATAL_ERROR "ARCH: ${ARCH} is not valid, set it to one of arch/ subdirectories")
endif()


if(NOT DEFINED TARGET_TYPE)
    message(FATAL_ERROR "TARGET_TYPE is not defined, set it to one of target/ subdirectories")
elseif(NOT EXISTS "${PROJECT_SOURCE_DIR}/target/${TARGET_TYPE}")
    message(FATAL_ERROR "TARGET_TYPE: ${TARGET_TYPE} is not valid, set it to one of target/ subdirectories")
endif()

string(TOUPPER ${ARCH} ARCH_UPPER)
string(TOUPPER ${TARGET_TYPE} TARGET_TYPE_UPPER)

if(NOT DEFINED TOOLCHAIN_PREFIX)
    message(FATAL_ERROR "TOOLCHAIN_PREFIX is not defined")
endif()

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/machine")
file(CREATE_LINK "${PROJECT_SOURCE_DIR}/arch/${ARCH}" "${PROJECT_SOURCE_DIR}/machine/arch" RESULT result SYMBOLIC)
file(CREATE_LINK "${PROJECT_SOURCE_DIR}/target/${TARGET_TYPE}" "${PROJECT_SOURCE_DIR}/machine/target" RESULT result SYMBOLIC)

include_directories("${PROJECT_SOURCE_DIR}")

# general sources are everything under core and libc
file(GLOB_RECURSE STUB_SOURCES core/*.c libc/*.c)

# we finished setting up, start describing the executable

add_executable(${PROJECT_NAME} ${STUB_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC
                           ARCH_${ARCH_UPPER}
                           TARGET_TYPE_${TARGET_TYPE_UPPER}
                           $<$<BOOL:${DEBUG}>:DEBUGMODE>
                           $<$<BOOL:${DEBUG_MALLOC}>:DEBUG_MALLOC>)

target_compile_options(${PROJECT_NAME} PUBLIC -fno-builtin $<$<BOOL:${DEBUG}>:-ggdb> "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=")
target_link_options(${PROJECT_NAME} PUBLIC -nostdlib -Wl,--no-dynamic-linker --entry=_start "-T${PROJECT_SOURCE_DIR}/linker.ld")

include("${PROJECT_SOURCE_DIR}/machine/arch/CMakeLists.txt")
include("${PROJECT_SOURCE_DIR}/machine/target/CMakeLists.txt")

if(NOT DEFINED LINKER_ENDIAN_FLAG)
    message(FATAL_ERROR "arch didn't define LINKER_ENDIAN_FLAG")
endif()

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_NAME}.bin)
add_custom_target(${PROJECT_NAME}.bin ALL DEPENDS ${PROJECT_NAME}.tmp)

add_custom_command(
    OUTPUT ${PROJECT_NAME}.tmp
    COMMAND ${TOOLCHAIN_PREFIX}objcopy --output-format=binary -j .init -j .text -j .data -j .bss -j .got ${PROJECT_NAME} ${PROJECT_NAME}.tmp
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.tmp ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}
)

# add gdb target xml
message(${LINKER_ENDIAN_FLAG})
add_custom_command(OUTPUT gdb_target_description.xml.o
                   COMMAND ${TOOLCHAIN_PREFIX}ld "${LINKER_ENDIAN_FLAG}" -r -b binary -o "${PROJECT_BINARY_DIR}/gdb_target_description.xml.o" gdb_target_description.xml
                   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/machine/arch"
                   COMMAND_EXPAND_LISTS # because of LINKER_ENDIAN_FLAG
                   DEPENDS "${CMAKE_SOURCE_DIR}/machine/arch/gdb_target_description.xml")
target_sources(${PROJECT_NAME} PUBLIC gdb_target_description.xml.o)

