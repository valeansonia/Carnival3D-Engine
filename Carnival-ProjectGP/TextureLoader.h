#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class TextureLoader {
public:
    static GLuint loadTexture(const std::string& path, bool flip = true);
};

#endif