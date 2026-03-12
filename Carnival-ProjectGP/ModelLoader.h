#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <map>
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    std::string texturePath;
    int textureIndex;

    Mesh() : VAO(0), VBO(0), EBO(0), textureIndex(-1) {}

    void setupMesh();
    void draw(unsigned int textureHandle);
    void cleanup();
};

class ModelLoader {
public:
    static std::vector<Mesh> loadModel(const std::string& path,std::vector<int>& modelTextures);
    static void cleanupMeshes(std::vector<Mesh>& meshes);

private:
    static void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const std::string& directory, glm::mat4 parentTransform);
    static Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory, glm::mat4 transform);

    static std::string extractDirectory(const std::string& filepath);
    static glm::mat4 assimpToGlmMatrix(aiMatrix4x4 from);

    static std::map<std::string, unsigned int> loadedTextures;
};