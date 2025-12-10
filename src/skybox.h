#ifndef SKYBOX_H
#define SKYBOX_H
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class skybox {
public:
    skybox();
    void init();
    void render(const float* view, const float* projection);
    unsigned int getTextureID() const { return textureID; }

    // Scroll control
    void setScrollSpeed(float speed);
    float getScrollSpeed() const;

private:
    unsigned int textureID;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;

    // Scrolling properties
    float scrollSpeed;
    float scrollOffset;

    unsigned int loadCubemap(std::vector<std::string> faces);
    unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels);
    void setupSkyboxVertices();
    void compileShaders();
};

#endif
