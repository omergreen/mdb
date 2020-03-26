set(ARCH_SHELLCODE_C_FLAGS "-nostdlib" "-nodefaultlibs" "-nostartfiles" "-fPIC" "-static")
set(ARCH_SHELLCODE_LD_FLAGS ${ARCH_SHELLCODE_C_FLAGS}  "--entry=_start" "-T${PROJECT_SOURCE_DIR}/machine/arch/linker.ld")
set(ARCH_SOURCES "")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_SYSROOT /home/me/x-tools/arm-unknown-linux-musleabi/arm-unknown-linux-musleabi/sysroot)
set(tools /home/me/x-tools/arm-unknown-linux-musleabi/bin/arm-unknown-linux-musleabi)
set(CMAKE_C_COMPILER ${tools}-gcc)
set(OBJCOPY ${tools}-objcopy)
# set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)

target_compile_options("${PROJECT_NAME}" PUBLIC "${ARCH_SHELLCODE_C_FLAGS}")
target_link_options("${PROJECT_NAME}" PUBLIC "${ARCH_SHELLCODE_LD_FLAGS}")
target_sources("${PROJECT_NAME}" PUBLIC "${ARCH_SOURCES}")

add_custom_command(
  OUTPUT "${PROJECT_NAME}.tmp"
  COMMAND ${OBJCOPY} --output-format=binary -j .text -j .data -j .bss ${PROJECT_NAME} ${PROJECT_NAME}.tmp
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.tmp ${PROJECT_NAME}.bin
  DEPENDS ${PROJECT_NAME}
)
