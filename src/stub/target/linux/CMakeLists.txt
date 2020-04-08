set(TARGET_SOURCES machine/target/syscall.c machine/target/libc.c machine/target/linux.c)

target_compile_definitions(${PROJECT_NAME} PUBLIC OVERRIDE_ARCH_CACHE_FLUSH COLORS)

target_sources(${PROJECT_NAME} PUBLIC ${TARGET_SOURCES})

