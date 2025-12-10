#pragma once
#include <GL/glew.h>
#include "treebase.h"

class TreeManager {
public:
    TreeManager();
    ~TreeManager();

    void generateTree(const std::string& preset, int iterations);

    void generateCustomTree(const std::string& axiom,
                            const std::string& rules,
                            int iterations,
                            float angle);

    void draw();

    void initialize();
    void cleanup();

    bool hasTree() const { return m_hasTree; }

    int getVertexCount() const { return m_vertexCount; }

    const std::vector<float>& getVertexData() const {
        return m_treeGenerator.getVertexData();
    }

    //wind parameter
    float getWindTime() const { return m_windTime; }
    void updateWind(float deltaTime) { m_windTime += deltaTime; }
private:
    void uploadToGPU();
    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    bool m_hasTree;
    bool m_glInitialized;
    TreeBase m_treeGenerator;

    //wind parameter
    float m_windTime;
};
