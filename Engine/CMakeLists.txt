project(soulsshard)

include(FetchContent)

find_package(Vulkan REQUIRED)


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

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
    GIT_TAG master
)

# Fetch TinyObjLoader
FetchContent_Declare(
  tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG v2.0.0rc13
)

FetchContent_MakeAvailable(
    fetch_vk_bootstrap
    glfw
    glm
    imgui
    imguizmo
    tinyobjloader
)


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
  ${imgui_SOURCE_DIR}/imstb_textedit.h
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

add_library(imguizmo
  ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
  ${imguizmo_SOURCE_DIR}/ImGuizmo.h
)


target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)

target_include_directories(imguizmo PUBLIC ${imguizmo_SOURCE_DIR})
target_link_libraries(imguizmo PUBLIC imgui)

file(GLOB_RECURSE SOURCES
		"src/*.cpp"
		"src/*.h")

# Create a library instead of an executable
add_library(${PROJECT_NAME} STATIC ${SOURCES})


include_directories(
    ${GLFW_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imguizmo_SOURCE_DIR}
)

# Link dependencies for the engine
target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap
    glfw
    glm
    imgui
    imguizmo
    tinyobjloader
    )
# Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
