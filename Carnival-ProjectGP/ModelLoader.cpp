#include "ModelLoader.h"
#include "TextureLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

std::map<std::string, unsigned int> ModelLoader::loadedTextures;
void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}

void Mesh::draw(unsigned int textureHandle) {
    if (textureHandle != -1) {
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,textureHandle);
    }
    else {
        
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::cleanup() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    VAO = VBO = EBO = 0;
}

std::vector<Mesh> ModelLoader::loadModel(const std::string& path,std::vector<int>& modelTextures) {
    std::vector<Mesh> meshes;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
    
    if (scene->HasMaterials()) {
		meshes.reserve(scene->mNumMaterials);
        for(unsigned int i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* material = scene->mMaterials[i];
            aiString str;
            
            if(material->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS) {
                std::string texturePath = extractDirectory(path) + "/" + std::string(str.C_Str());
                unsigned int textureID = TextureLoader::loadTexture(texturePath.c_str());
                modelTextures.push_back(textureID);
            }
            else {
				modelTextures.push_back(-1);
            }
        }
    }

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        return meshes;
    }

    meshes.reserve(scene->mNumMeshes);
    std::string directory = extractDirectory(path);
    processNode(scene->mRootNode, scene, meshes, directory, glm::mat4(1.0f));

    for (size_t i = 0; i < meshes.size(); i++) {
        meshes[i].setupMesh();
    }

    return meshes;
}

glm::mat4 ModelLoader::assimpToGlmMatrix(aiMatrix4x4 from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const std::string& directory, glm::mat4 parentTransform) {
    glm::mat4 nodeTransform = parentTransform * assimpToGlmMatrix(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, directory, nodeTransform));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes, directory, nodeTransform);
    }
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory, glm::mat4 transform) {
    Mesh result;
    result.textureIndex = mesh->mMaterialIndex;
                        
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        glm::vec4 pos = transform * glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        vertex.position = glm::vec3(pos);

        if (mesh->HasNormals()) {
            vertex.normal = glm::mat3(glm::transpose(glm::inverse(transform))) * glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        if (mesh->HasVertexColors(0)) {
            vertex.color = glm::vec3(mesh->mColors[0][i].r,
                mesh->mColors[0][i].g,
                mesh->mColors[0][i].b);
        }
        else {
			vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        }

        if (mesh->HasTextureCoords(0)) {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
			vertex.texCoords = glm::vec2(-1.0f, -1.0f);
        }
 
        result.vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            result.indices.push_back(face.mIndices[j]);
        }
    }

    std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices\n";

    return result;
}

std::string ModelLoader::extractDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    return (lastSlash != std::string::npos) ? filepath.substr(0, lastSlash) : ".";
}

void ModelLoader::cleanupMeshes(std::vector<Mesh>& meshes) {
    for (auto& mesh : meshes) mesh.cleanup();
    meshes.clear();
}