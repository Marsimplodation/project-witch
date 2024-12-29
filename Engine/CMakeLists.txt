project(soulsshard)

include(FetchContent)
include(FetchContent)
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        v1.3.302 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
# Fetch GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8 
)
# Fetch EnTT from GitHub
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG master  # You can specify a release tag here, if needed
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1 # Replace with the desired version tag or branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap glfw entt glm)
file(GLOB_RECURSE SOURCES
		"src/*.cpp"
		"src/*.h")

# Create a library instead of an executable
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Link dependencies for the engine
target_link_libraries(${PROJECT_NAME} PRIVATE vk-bootstrap::vk-bootstrap glfw glm)

# Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
# Include the EnTT header files
target_include_directories(${PROJECT_NAME} PUBLIC ${entt_SOURCE_DIR}/src)
