#ifndef INPUT_HANLDER_H
#define INPUT_HANLDER_H
#include "GLFW/glfw3.h"
#include "../types/types.h"
#include "KeyDefines.h"
#include <vector>

struct InputHandler {
    void init(GLFWwindow * window);
    void update();
    bool isKeyPressed(int key);
    bool isKeyPressedOnce(int key);
    bool isMousePressed(int key);
    void captureMouse();
    void releaseMouse();
    glm::vec2 getMouseDelta();
private:
    std::vector<u32> blockedKeys;
    std::vector<bool> keysPressed=std::vector<bool>(GLFW_KEY_LAST);
    bool ready = false;
};
#endif // !INPUT_HANLDER_H
