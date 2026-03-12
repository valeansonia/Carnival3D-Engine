#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Camera.h"
#include "InputHandler.h"
#include "TextureLoader.h"
#include "SceneManager.h"
#include "stb_image.h"

int glWindowWidth = 1400;
int glWindowHeight = 900;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLuint shaderProgram;

float carouselRotation = 0.0f;

std::string readShaderFile(std::string fileName) {
    std::ifstream shaderFile(fileName);
    if (!shaderFile.is_open()) {
        std::cerr << "ERROR: Could not open shader file: " << fileName << std::endl;
        return "";
    }
    std::stringstream shaderStringStream;
    shaderStringStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStringStream.str();
}

void shaderCompileLog(GLuint shaderId) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        std::cout << "Shader compilation error:\n" << infoLog << std::endl;
    }
}

void shaderLinkLog(GLuint shaderProgram) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader linking error:\n" << infoLog << std::endl;
    }
}

GLuint initBasicShader(std::string vertexFile, std::string fragmentFile) {
    std::string vertexCode = readShaderFile(vertexFile);
    std::string fragmentCode = readShaderFile(fragmentFile);

    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    shaderCompileLog(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    shaderCompileLog(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    shaderLinkLog(program);

    return program;
}

void renderScene(SceneManager* sceneManager,GLuint shadowMap,const glm::mat4& lightSpace) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.05f, 0.1f, 0.15f, 1.0f);

    glViewport(0, 0, retina_width, retina_height);
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), (int)sceneManager->lights.size());
   
    glUniform1i(glGetUniformLocation(shaderProgram, "textureUnit"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)retina_width / retina_height, 0.1f, 150.0f);
    glm::mat4 view = sceneManager->camera->getViewMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(glm::mat4(1.0f))));

    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint normalMatrixLoc = glGetUniformLocation(shaderProgram, "normalMatrix");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3f(viewPosLoc, sceneManager->camera->position.x,
        sceneManager->camera->position.y,
		sceneManager->camera->position.z);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpace"), 1, GL_FALSE, glm::value_ptr(lightSpace));

    // Pass lights to shader
    GLint numLightsLoc = glGetUniformLocation(shaderProgram, "numLights");
    glUniform1i(numLightsLoc, sceneManager->lights.size());

    //asta adaug
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureUnit"), 0);

    for (int i = 0; i < sceneManager->lights.size() && i < 8; i++) {
        std::string lightPos = "lightPositions[" + std::to_string(i) + "]";
        std::string lightColor = "lightColors[" + std::to_string(i) + "]";
        std::string lightIntensity = "lightIntensities[" + std::to_string(i) + "]";

        glUniform3fv(glGetUniformLocation(shaderProgram, lightPos.c_str()), 1,
            glm::value_ptr(sceneManager->lights[i].position));
        glUniform3fv(glGetUniformLocation(shaderProgram, lightColor.c_str()), 1,
            glm::value_ptr(sceneManager->lights[i].color));
        glUniform1f(glGetUniformLocation(shaderProgram, lightIntensity.c_str()),
            sceneManager->lights[i].intensity);
    }

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    for (auto obj : sceneManager->gameObjects) {
		if (!obj) continue;
        glm::mat4 model = obj->getModelMatrix();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glm::mat3 objNormalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(objNormalMatrix));

        if (!obj->meshes.empty()) {
            for (auto& mesh : obj->meshes) {
                mesh.draw(obj->textures[mesh.textureIndex]);
            }
        }
    }

    glBindVertexArray(0);
}


void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glWindowWidth = width;
    glWindowHeight = height;
    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    printf("Window resized to %d x %d\n", width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "🎪 CARNIVAL 3D", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyCallback);
    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1); // Vsync

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version: %s\n", version);

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void cleanup(SceneManager* sceneManager) {
    if (sceneManager) delete sceneManager;
    if (glWindow) glfwDestroyWindow(glWindow);
    glfwTerminate();
}

void initShadowMap(GLuint& fbo,GLuint& shadowMap) {
    static GLuint shadowMapFBO;
    GLuint shadowMapTex;
    static const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &shadowMapTex);
    glBindTexture(GL_TEXTURE_2D, shadowMapTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTex, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	fbo = shadowMapFBO;
	shadowMap = shadowMapTex;
}

void renderShadowMap(SceneManager* sceneManager, GLuint depthShader,GLuint shadowMapFBO,glm::mat4& lightSpace) {
    float near_plane = 0.1f, far_plane = 50.0f;
    glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);

    glm::vec3 lightPos = glm::vec3(0, 5, 20);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glUseProgram(depthShader);
    glUniformMatrix4fv(glGetUniformLocation(depthShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_FRONT);

    for (auto obj : sceneManager->gameObjects) {
        if (!obj) continue;
        glm::mat4 model = obj->getModelMatrix();
        glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

        if (!obj->meshes.empty()) {
            for (auto& mesh : obj->meshes) {
                mesh.draw(obj->textures[mesh.textureIndex]);
            }
        }
    }

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    lightSpace = lightSpaceMatrix;
}

GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    stbi_set_flip_vertically_on_load(false);
    
    for (unsigned int i = 0; i < faces.size(); i++) {
        int width, height, nrChannels;
        if(i == 2) stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
        }
        stbi_set_flip_vertically_on_load(false);
    }
    stbi_set_flip_vertically_on_load(true);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderSkybox(GLuint skyboxVAO, GLuint skyboxShader, GLuint cubemapTexture, glm::mat4 view, glm::mat4 projection) {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxShader);

    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(skyboxShader, "skybox"),0);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

const float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
};

int main(int argc, const char* argv[]) {
    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    shaderProgram = initBasicShader("vertexShader.glsl", "fragmentShader.glsl");
	GLuint depthShader = initBasicShader("vertexShadow.glsl", "fragmentShadow.glsl");
    GLuint skyboxShader = initBasicShader("cubemapVertexShader.glsl", "cubemapFragmentShader.glsl");

    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    SceneManager* sceneManager = new SceneManager();
    sceneManager->initScene();

    std::vector<std::string> faces = {
    "./assets/cubemap/left.jpg","./assets/cubemap/right.jpg","./assets/cubemap/top.jpg",
    "./assets/cubemap/bottom.jpg", "./assets/cubemap/front.jpg", "./assets/cubemap/back.jpg"
    };
    GLuint cubemapTexture = loadCubemap(faces);

    std::cout << "\n=== CARNIVAL 3D ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/A/S/D - Move camera" << std::endl;
    std::cout << "  SPACE/CTRL - Up/Down" << std::endl;
    std::cout << "  Right Mouse + Move - Rotate view" << std::endl;
    std::cout << "  ESC - Exit" << std::endl << std::endl;

    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	GLuint shadowMapFBO, shadowMap;
    initShadowMap(shadowMapFBO, shadowMap);
	glm::mat4 lightSpace;
    while (!glfwWindowShouldClose(glWindow)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
		std::cout << sceneManager->camera->position.x << " " << sceneManager->camera->position.y << " " << sceneManager->camera->position.z << "\n";
        InputHandler::handleInput(glWindow, sceneManager->camera, deltaTime);
        sceneManager->updateScene(deltaTime);

		renderShadowMap(sceneManager, depthShader, shadowMapFBO, lightSpace);

        static bool wireframe = false;
        static bool held = false;
        if (glfwGetKey(glWindow, GLFW_KEY_Q) == GLFW_PRESS) {
            if (!held) {
                wireframe = !wireframe;
                if (wireframe)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            held = true;
        }
        else {
            held = false;
        }

        renderScene(sceneManager,shadowMap, lightSpace);

        glm::mat4 view = sceneManager->camera->getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)retina_width / retina_height, 0.1f, 150.0f);

        glm::mat4 staticView = glm::mat4(glm::mat3(view));

        renderSkybox(skyboxVAO, skyboxShader, cubemapTexture, staticView, projection);

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup(sceneManager);
    return 0;
}