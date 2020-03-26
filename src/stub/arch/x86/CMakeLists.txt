set(ARCH_SHELLCODE_C_FLAGS "-m32" "--static-pie" "-nostdlib" "-nodefaultlibs" "-nolibc" "-nostartfiles" "-fpic")
set(ARCH_SHELLCODE_LD_FLAGS "-m32" "--static-pie" "-nostdlib" "-nodefaultlibs" "-nolibc" "-nostartfiles" "--entry=_start")
set(ARCH_SOURCES "")

target_compile_options("${PROJECT_NAME}" PUBLIC "${ARCH_SHELLCODE_C_FLAGS}")
target_link_options("${PROJECT_NAME}" PUBLIC "${ARCH_SHELLCODE_LD_FLAGS}")
target_sources("${PROJECT_NAME}" PUBLIC "${ARCH_SOURCES}")

add_custom_command(
  OUTPUT "${PROJECT_NAME}.tmp"
  COMMAND objcopy --output-format=binary -j .text -j .bss ${PROJECT_NAME} ${PROJECT_NAME}.tmp
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.tmp ${PROJECT_NAME}.bin
  DEPENDS ${PROJECT_NAME}
)
