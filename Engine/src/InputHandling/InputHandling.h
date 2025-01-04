#ifndef INPUT_HANLDER_H
#define INPUT_HANLDER_H
#include "GLFW/glfw3.h"
#include "../types/types.h"
#include "KeyDefines.h"

struct InputHandler {
    void init(GLFWwindow * window);
    void update();
    bool isKeyPressed(int key);
    bool isMousePressed(int key);
    void captureMouse();
    void releaseMouse();
    glm::vec2 getMouseDelta();
private:
    bool ready = false;
};
#endif // !INPUT_HANLDER_H
