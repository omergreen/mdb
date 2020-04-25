set(TARGET_SOURCES machine/target/rtthread.c)

target_compile_definitions(${PROJECT_NAME} PUBLIC COLORS)

target_sources(${PROJECT_NAME} PUBLIC ${TARGET_SOURCES})

