#ifndef SKYBOX_H
#define SKYBOX_H

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
// #include <OpenGL/gl.h>

#ifdef _WIN32
#include <GL/gl.h>        // Windows + MinGW
#elif __APPLE__
#include <OpenGL/gl.h>    // macOS
#else
#include <GL/gl.h>        // Linux fallback
#endif


class skybox {
public:
    skybox();
    void init();
    void render(const float* view, const float* projection);
    unsigned int getTextureID() const { return textureID; }

private:
    unsigned int textureID;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;

    unsigned int loadCubemap(std::vector<std::string> faces);
    unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels);
    void setupSkyboxVertices();
    void compileShaders();
};

#endif
