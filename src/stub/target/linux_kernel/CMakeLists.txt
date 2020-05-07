set(TARGET_SOURCES ${CMAKE_CURRENT_LIST_DIR}/linux_kernel.c)

if(NOT DEFINED KERNEL_DIR)
    message(FATAL_ERROR "KERNEL_DIR must be defined")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC COLORS)

target_sources(${PROJECT_NAME} PUBLIC ${TARGET_SOURCES})

FILE(RELATIVE_PATH BUILD_TO_SOURCE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR})
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Kbuild "obj-m := kernel_module.o \n kernel_module-y := ${BUILD_TO_SOURCE}/kernel_module.o")
add_custom_command(OUTPUT kernel_module.ko
                   COMMAND ${MAKE} -C ${KERNEL_DIR} modules M=${CMAKE_CURRENT_BINARY_DIR} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN_PREFIX}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/kernel_module.c VERBATIM)

target_sources(${PROJECT_NAME} PUBLIC kernel_module.ko)

