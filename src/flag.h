#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct Particle {
    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 acceleration;
    glm::vec3 normal;
    bool fixed;
    float mass;
};

struct Spring {
    int p1Index;
    int p2Index;
    float restLength;
    float stiffness;
    float damping;
};

class Flag {
public:
    Flag();
    ~Flag();

    void initialize(int width, int height, float spacing, const glm::vec3& anchorPos);
    void update(float deltaTime);
    void render();
    void renderPole();
    void cleanup();

    void setWindForce(const glm::vec3& wind) { m_windForce = wind; }
    void setGravity(const glm::vec3& gravity) { m_gravity = gravity; }

    const std::vector<float>& getVertexData() const { return m_vertexData; }
    int getVertexCount() const { return m_particles.size() * 6; } // 2 triangles per quad

    GLuint getVAO() const { return m_vao; }
    GLuint getVBO() const { return m_vbo; }

    //Pole
    GLuint getPoleVAO() const { return m_poleVao; }
    GLuint getPoleVBO() const { return m_poleVbo; }
    int getPoleVertexCount() const { return m_poleVertexCount; }

private:
    void createMesh(int width, int height, float spacing, const glm::vec3& anchorPos);
    void addSpring(int p1, int p2, float stiffness);

    void satisfyConstraints();
    void updateNormals();
    void updateVertexBuffer();

    int getIndex(int x, int y) const { return y * m_width + x; }

    std::vector<Particle> m_particles;
    std::vector<Spring> m_springs;
    std::vector<float> m_vertexData;

    int m_width;
    int m_height;
    float m_spacing;

    glm::vec3 m_windForce;
    glm::vec3 m_gravity;

    float m_springStiffness;
    float m_springDamping;
    float m_maxStretch;

    GLuint m_vao;
    GLuint m_vbo;
    bool m_initialized;

    //For pole stuff
    void createPole(float poleHeight, float poleRadius);
    std::vector<float> m_poleVertexData;
    GLuint m_poleVao;
    GLuint m_poleVbo;
    int m_poleVertexCount;
};
