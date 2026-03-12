#include "SceneManager.h"
#include "ModelLoader.h"
#include <filesystem>
#include <iostream>
#include <cmath>

SceneManager::SceneManager() {
    camera = new Camera(glm::vec3(0, 5, 20));

    lights.push_back({ glm::vec3(-15, 4, 15), glm::vec3(1.0f, 1.0f, 0.9f), 1.5f, 0 });
    lights.push_back({ glm::vec3(15, 4, 15), glm::vec3(1.0f, 1.0f, 0.9f), 1.5f, 0 });
    lights.push_back({ glm::vec3(3, 3, 3), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0 });
    lights.push_back({ glm::vec3(-3, 3, 0), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0 });
    lights.push_back({ glm::vec3(0, 3, 3), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0 });
}

SceneManager::~SceneManager() {
    cleanup();
}

GameObject* SceneManager::createCube(glm::vec3 position, glm::vec3 scale, std::string name) {
    GameObject* obj = new GameObject();
    obj->position = position;
    obj->scale = scale;
    obj->rotation = glm::vec3(0.0f);
    obj->name = name;

    float vertices[] = {
        // pozitii            // culori
        -0.5f, -0.5f, -0.5f, 0.8f, 0.2f, 0.2f,
         0.5f, -0.5f, -0.5f, 0.8f, 0.2f, 0.2f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.3f, 0.3f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.3f, 0.3f,

        -0.5f, -0.5f,  0.5f, 0.2f, 0.8f, 0.2f,
         0.5f, -0.5f,  0.5f, 0.2f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f, 0.3f, 1.0f, 0.3f,
        -0.5f,  0.5f,  0.5f, 0.3f, 1.0f, 0.3f,

        -0.5f,  0.5f,  0.5f, 0.2f, 0.2f, 0.8f,
        -0.5f,  0.5f, -0.5f, 0.2f, 0.2f, 0.8f,
        -0.5f, -0.5f, -0.5f, 0.3f, 0.3f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.3f, 0.3f, 1.0f,

         0.5f,  0.5f,  0.5f, 0.8f, 0.8f, 0.2f,
         0.5f,  0.5f, -0.5f, 0.8f, 0.8f, 0.2f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.3f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.3f,

        -0.5f, -0.5f, -0.5f, 0.8f, 0.2f, 0.8f,
         0.5f, -0.5f, -0.5f, 0.8f, 0.2f, 0.8f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.3f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.3f, 1.0f,

        -0.5f,  0.5f, -0.5f, 0.2f, 0.8f, 0.8f,
         0.5f,  0.5f, -0.5f, 0.2f, 0.8f, 0.8f,
         0.5f,  0.5f,  0.5f, 0.3f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.3f, 1.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 6, 5, 4, 7, 6,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 20, 23, 22
    };

    obj->indexCount = 36;

    glGenVertexArrays(1, &obj->VAO);
    glGenBuffers(1, &obj->VBO);
    glGenBuffers(1, &obj->EBO);

    glBindVertexArray(obj->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return obj;
}

GameObject* SceneManager::createCylinder(glm::vec3 position, float radius, float height, std::string name) {
    GameObject* obj = new GameObject();
    obj->position = position;
    obj->scale = glm::vec3(1.0f);
    obj->rotation = glm::vec3(0.0f);
    obj->name = name;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    int segments = 32;
    float colorR = 1.0f, colorG = 0.84f, colorB = 0.0f;

    // Top center
    vertices.push_back(0.0f);
    vertices.push_back(height);
    vertices.push_back(0.0f);
    vertices.push_back(colorR);
    vertices.push_back(colorG);
    vertices.push_back(colorB);

    // Bottom center
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(colorR);
    vertices.push_back(colorG);
    vertices.push_back(colorB);

    // Side vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Top side
        vertices.push_back(x);
        vertices.push_back(height);
        vertices.push_back(z);
        vertices.push_back(colorR * 0.8f);
        vertices.push_back(colorG * 0.8f);
        vertices.push_back(colorB * 0.8f);

        // Bottom side
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
        vertices.push_back(colorR * 0.6f);
        vertices.push_back(colorG * 0.6f);
        vertices.push_back(colorB * 0.6f);
    }

    // Top cap
    for (int i = 0; i < segments; ++i) {
        indices.push_back(0);
        indices.push_back(2 + i * 2);
        indices.push_back(2 + (i + 1) * 2);
    }

    // Bottom cap
    for (int i = 0; i < segments; ++i) {
        indices.push_back(1);
        indices.push_back(3 + (i + 1) * 2);
        indices.push_back(3 + i * 2);
    }

    // Side faces
    for (int i = 0; i < segments; ++i) {
        int topLeft = 2 + i * 2;
        int topRight = 2 + (i + 1) * 2;
        int bottomLeft = 3 + i * 2;
        int bottomRight = 3 + (i + 1) * 2;

        indices.push_back(topLeft);
        indices.push_back(bottomLeft);
        indices.push_back(topRight);

        indices.push_back(topRight);
        indices.push_back(bottomLeft);
        indices.push_back(bottomRight);
    }

    obj->indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &obj->VAO);
    glGenBuffers(1, &obj->VBO);
    glGenBuffers(1, &obj->EBO);

    glBindVertexArray(obj->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return obj;
}

GameObject* SceneManager::createModelFromFile(const std::string& filePath, glm::vec3 position, glm::vec3 scale, const std::string& name) {
    std::cout << "\n [createModelFromFile] Loading: " << filePath << "\n";
    std::vector<int> modelTextures;
    std::vector<Mesh> modelMeshes = ModelLoader::loadModel(filePath,modelTextures);

    if (modelMeshes.empty()) {
        std::cerr << " ERROR: Failed to load model: " << filePath << std::endl;
        return nullptr;
    }

    GameObject* obj = new GameObject();
    obj->position = position;
    obj->scale = scale;
    obj->rotation = glm::vec3(0.0f);
    obj->name = name;
    obj->meshes = std::move(modelMeshes);
    obj->textures = std::move(modelTextures);
    std::cout << " [createModelFromFile] Successfully loaded " << obj->meshes.size() << " meshes into " << name << "\n";

    return obj;
}

void printCurrentDirectoryContents() {
	using namespace std::filesystem;
    try {
        path currentPath = current_path();

        std::cout << "\n[DEBUG] Directorul curent de lucru: " << currentPath << "\n";

        for (const auto& entry : directory_iterator(currentPath)) {
            std::string type = entry.is_directory() ? "[DIR] " : "[FILE]";
            std::cout << type << " " << entry.path().filename().string() << "\n";
        }
    }
    catch (const filesystem_error& e) {
        std::cerr << "Eroare la accesarea directorului: " << e.what() << "\n";
    }
}

void SceneManager::initScene() {
    aiScene* scene;
    std::string modelPath = "./assets/models/amusement_park/scene.gltf";

    GameObject* carousel = createModelFromFile(modelPath, glm::vec3(0, 0, 0), glm::vec3(1.0f), "Carousel");

    if (carousel != nullptr) {
        
        gameObjects.push_back(carousel);
        std::cout << "SUCCESS: Carousel added to scene\n";
    }
    else {

        GameObject* fallback = createCylinder(glm::vec3(0, 0, 0), 3.0f, 4.0f, "Carousel");
        if (fallback) gameObjects.push_back(fallback);
    }
}

void SceneManager::updateScene(float deltaTime) {
    
}

void SceneManager::cleanup() {
    for (auto obj : gameObjects) {
        if (obj) {
            delete obj;
        }
    }
    gameObjects.clear();

    if (camera) {
        delete camera;
        camera = nullptr;
    }
}