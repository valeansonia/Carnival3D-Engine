#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <string>
#include <vector>
#include  "ModelLoader.h"

class GameObject {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    GLuint VAO, VBO, EBO;
    int indexCount;
    std::vector<int> textures;
    std::string name;
	std::vector<Mesh> meshes;
    // Constructor
    GameObject()
        : position(0.0f), rotation(0.0f), scale(1.0f),
        VAO(0), VBO(0), EBO(0), textures({}), indexCount(0), name("") {
    }

    glm::mat4 getModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};

#endif