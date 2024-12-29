# Project name
project(witch)


file(GLOB_RECURSE SOURCES
		"src/*.cpp"
		"src/*.h")
# Add executable
add_executable(${PROJECT_NAME}  ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE soulsshard)
