set(ARCH_SHELLCODE_C_FLAGS "-nostdlib" "-nodefaultlibs" "-nostartfiles" "-fno-builtin" "-fPIE" "-static")
set(ARCH_SHELLCODE_LD_FLAGS ${ARCH_SHELLCODE_C_FLAGS}  "--entry=_start" "-T${PROJECT_SOURCE_DIR}/machine/arch/linker.ld")
file(GLOB ARCH_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.c")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)

target_compile_options("${PROJECT_NAME}" PUBLIC "${ARCH_SHELLCODE_C_FLAGS}")
target_link_options("${PROJECT_NAME}" PUBLIC "${ARCH_SHELLCODE_LD_FLAGS}")
target_sources("${PROJECT_NAME}" PUBLIC "${ARCH_SOURCES}")

add_custom_command(
  OUTPUT "${PROJECT_NAME}.tmp"
  COMMAND ${OBJCOPY} --output-format=binary -j .text -j .data -j .bss -j .got ${PROJECT_NAME} ${PROJECT_NAME}.tmp
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.tmp ${PROJECT_NAME}.bin
  DEPENDS ${PROJECT_NAME}
)

