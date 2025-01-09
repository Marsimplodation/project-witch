#include "InputHandling.h"
#include "GLFW/glfw3.h"
#include <GLFW/glfw3.h>
#include <vector>

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

bool InputHandler::isKeyPressedOnce(int key) {
    if(keysPressed[key]) return false;
    keysPressed[key] = true;
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

//give glfw 10 frames to catch up on input
int counter = 0;
void InputHandler::update(){
    double x, y;
    glfwGetCursorPos(state.window, &x,&y);
    state.mousePosition = {x,y};
    if(!ready) {
        ready = (counter++) >= 10;
    }

    for (int key = 0; key < GLFW_KEY_LAST; key++) {
        bool released = (glfwGetKey(state.window, key) == GLFW_RELEASE);
        keysPressed[key] = !released;
    }
}
glm::vec2 InputHandler::getMouseDelta() {
    double x, y;
    glfwGetCursorPos(state.window, &x,&y);
    glm::vec2 delta = {x-state.mousePosition.x, y-state.mousePosition.y};
    state.mousePosition = {x,y};
    if(!ready) return glm::vec2(0);
    return delta;
}
