#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    Camera(glm::vec3 pos = glm::vec3(0, 5, 20))
        : position(pos), target(0, 2, 0), up(0, 1, 0) {
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, target, up);
    }

    void moveForward(float speed) {
        glm::vec3 direction = glm::normalize(target - position);
        position += direction * speed;
        target += direction * speed;
    }

    void moveRight(float speed) {
        glm::vec3 direction = glm::normalize(glm::cross(target - position, up));
        position += direction * speed;
        target += direction * speed;
    }

    void moveUp(float speed) {
        position += up * speed;
        target += up * speed;
    }

    void rotateAroundTarget(float angleX, float angleY) {
        glm::vec3 dir = position - target;
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation = glm::rotate(rotation, glm::radians(angleY), up);
        rotation = glm::rotate(rotation, glm::radians(angleX), glm::normalize(glm::cross(up, dir)));
        position = target + glm::vec3(rotation * glm::vec4(dir, 0.0f));
    }
};

#endif