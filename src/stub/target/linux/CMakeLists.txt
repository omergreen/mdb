set(TARGET_SOURCES "machine/target/syscall.c" "machine/target/libc.c" "machine/target/linux.c")

target_sources("${PROJECT_NAME}" PUBLIC "${TARGET_SOURCES}")
