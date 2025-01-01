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


FetchContent_Declare(
    imgui
    GIT_REPOSITORY GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6-docking
)

FetchContent_MakeAvailable(fetch_vk_bootstrap glfw entt glm imgui)
find_package(Vulkan REQUIRED)

add_library(imgui
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui.h
  ${imgui_SOURCE_DIR}/imconfig.h
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_internal.h
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imstb_rectpack.h
  ${imgui_SOURCE_DIR}/imstb_textedit.h
  ${imgui_SOURCE_DIR}/imstb_truetype.h
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)
file(GLOB_RECURSE SOURCES
		"src/*.cpp"
		"src/*.h")

# Create a library instead of an executable
add_library(${PROJECT_NAME} STATIC ${SOURCES})


include_directories(
    ${GLFW_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
)

# Link dependencies for the engine
target_link_libraries(${PROJECT_NAME} PRIVATE vk-bootstrap::vk-bootstrap glfw glm imgui)

# Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
# Include the EnTT header files
target_include_directories(${PROJECT_NAME} PUBLIC ${entt_SOURCE_DIR}/src)
