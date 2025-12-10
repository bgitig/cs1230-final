#ifndef ROCKGENERATOR_H
#define ROCKGENERATOR_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

class RockGenerator {
public:
    RockGenerator();
    ~RockGenerator();

    void initialize();
    void generateRock(int detail = 2);
    void cleanup();

    void draw();

    bool hasRock() const { return m_hasRock; }
    int getVertexCount() const { return m_vertexCount; }
    void calculateSmoothNormals();

    std::vector<float> getVertexDataSimple() const;
    void perturbVertices();

    const std::vector<float>& getFullVertexData() const { return m_vertexData; }

private:
    void generateIcosphere(int subdivisions);
    void subdivideMesh();
    void calculateTangents();

    std::vector<float> m_vertexData;
    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    bool m_hasRock;
    bool m_initialized;

    void smoothNormalsAfterSubdivision();
};

#endif
