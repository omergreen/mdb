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

if(NOT DEFINED TOOLCHAIN_PREFIX)
    message(FATAL_ERROR "TOOLCHAIN_PREFIX is not defined")
endif()

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/machine")
file(CREATE_LINK "${PROJECT_SOURCE_DIR}/arch/${ARCH}" "${PROJECT_SOURCE_DIR}/machine/arch" RESULT result SYMBOLIC)
file(CREATE_LINK "${PROJECT_SOURCE_DIR}/target/${TARGET_TYPE}" "${PROJECT_SOURCE_DIR}/machine/target" RESULT result SYMBOLIC)

include_directories("${PROJECT_SOURCE_DIR}")

# general sources are everything under core and libc
file(GLOB_RECURSE STUB_SOURCES core/*.c libc/*.c)
add_executable(${PROJECT_NAME} ${STUB_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC
                           ARCH=${ARCH}
                           TARGET_TYPE=${TARGET_TYPE}
                           $<$<BOOL:${DEBUG}>:DEBUGMODE>
                           $<$<BOOL:${DEBUG_MALLOC}>:DEBUG_MALLOC>)

# add_subdirectory("${PROJECT_SOURCE_DIR}/machine")
include("${PROJECT_SOURCE_DIR}/machine/arch/CMakeLists.txt")
include("${PROJECT_SOURCE_DIR}/machine/target/CMakeLists.txt")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_NAME}.bin)
add_custom_target(${PROJECT_NAME}.bin ALL DEPENDS ${PROJECT_NAME}.tmp)

