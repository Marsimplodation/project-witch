#include "InputHandling.h"
#include "GLFW/glfw3.h"
#include <GLFW/glfw3.h>

namespace {
    struct {
        GLFWwindow *window;
        glm::vec2 mousePosition;
    } state;
}

void InputHandler::init(GLFWwindow *window) {
    state.window = window;
    double x, y;
    glfwGetCursorPos(state.window, &x,&y);
    state.mousePosition = {x,y};
}
bool InputHandler::isKeyPressed(int key) {
    return glfwGetKey(state.window, key) == GLFW_PRESS || glfwGetMouseButton(state.window, key) == GLFW_PRESS;
}
void InputHandler::captureMouse() {
    if(glfwGetInputMode(state.window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
    glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputHandler::releaseMouse() {
    if(glfwGetInputMode(state.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void InputHandler::update(){
    double x, y;
    glfwGetCursorPos(state.window, &x,&y);
    state.mousePosition = {x,y};
}
glm::vec2 InputHandler::getMouseDelta() {
    double x, y;
    glfwGetCursorPos(state.window, &x,&y);
    glm::vec2 delta = {x-state.mousePosition.x, y-state.mousePosition.y};
    state.mousePosition = {x,y};
    return delta;
}
