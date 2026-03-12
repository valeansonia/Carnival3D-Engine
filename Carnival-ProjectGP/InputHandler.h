#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <GLFW/glfw3.h>
#include "Camera.h"

class InputHandler {
public:
    static void handleInput(GLFWwindow* window, Camera* camera, float deltaTime);
};

#endif