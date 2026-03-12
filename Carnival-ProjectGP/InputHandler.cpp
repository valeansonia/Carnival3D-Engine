#include "InputHandler.h"

void InputHandler::handleInput(GLFWwindow* window, Camera* camera, float deltaTime) {
    float speed = 0.05f;

    if (glfwGetKey(window, GLFW_KEY_W))
        camera->moveForward(speed);
    if (glfwGetKey(window, GLFW_KEY_S))
        camera->moveForward(-speed);
    if (glfwGetKey(window, GLFW_KEY_A))
        camera->moveRight(-speed);
    if (glfwGetKey(window, GLFW_KEY_D))
        camera->moveRight(speed);
    if (glfwGetKey(window, GLFW_KEY_SPACE))
        camera->moveUp(speed);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
        camera->moveUp(-speed);

    static double lastX = 0, lastY = 0;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
        float xoffset = (xpos - lastX) * 0.1f;
        float yoffset = (lastY - ypos) * 0.1f;
        camera->rotateAroundTarget(yoffset, xoffset);
    }

    lastX = xpos;
    lastY = ypos;
}