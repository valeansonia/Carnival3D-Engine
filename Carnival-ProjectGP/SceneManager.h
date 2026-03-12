#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "GameObject.h"
#include "Camera.h"
#include "ModelLoader.h"

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    int type;
};

class SceneManager {
public:
    std::vector<GameObject*> gameObjects;
    std::vector<Light> lights;
    Camera* camera;
    SceneManager();
    ~SceneManager();

    void initScene();
    void updateScene(float deltaTime);
    void cleanup();

private:
    GameObject* createCube(glm::vec3 position, glm::vec3 scale, std::string name);
    GameObject* createCylinder(glm::vec3 position, float radius, float height, std::string name);
    GameObject* createModelFromFile(const std::string& filePath, glm::vec3 position, glm::vec3 scale, const std::string& name);
};

#endif