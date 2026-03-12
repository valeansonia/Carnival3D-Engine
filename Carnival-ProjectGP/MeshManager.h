#ifndef MESHGENERATOR_H
#define MESHGENERATOR_H

#include <vector>
#include <glm/glm.hpp>
#include "GameObject.h"

class MeshGenerator {
public:
    // Creaza o sfera
    static GameObject* createSphere(float radius, int sectorCount, int stackCount) {
        GameObject* obj = new GameObject();
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float x, y, z;
        for (int i = 0; i <= stackCount; ++i) {
            float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
            float xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectorCount; ++j) {
                float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
                vertices.push_back(x / radius); vertices.push_back(y / radius); vertices.push_back(z / radius);
                vertices.push_back((float)j / sectorCount); vertices.push_back((float)i / stackCount);
            }
        }

        setupMesh(obj, vertices, indices);
        return obj;
    }

    // Create a cub
    static GameObject* createCube(float size) {
        GameObject* obj = new GameObject();
        std::vector<float> vertices = {
            -size, -size,  size,   0, 0, 1,   0, 0,
             size, -size,  size,   0, 0, 1,   1, 0,
             size,  size,  size,   0, 0, 1,   1, 1,
            -size,  size,  size,   0, 0, 1,   0, 1,
            // Continue for other faces...
        };
        std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };
        setupMesh(obj, vertices, indices);
        return obj;
    }

private:
    static void setupMesh(GameObject* obj, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
        obj->indexCount = indices.size();
        glGenVertexArrays(1, &obj->VAO);
        glGenBuffers(1, &obj->VBO);
        glGenBuffers(1, &obj->EBO);

        glBindVertexArray(obj->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        int stride = 11 * sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // Normal (location 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Color (location 2)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // TexCoords (location 3)
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);
    }
};

#endif