#include "flag.h"
#include <iostream>
#include <cmath>

Flag::Flag()
    : m_width(0), m_height(0), m_spacing(0.0f),
    m_windForce(2.0f, 0.0f, 1.0f),
    m_gravity(0.0f, 0.0f, -9.8f),
    m_springStiffness(50.0f),
    m_springDamping(0.25f),
    m_maxStretch(1.1f),
    m_vao(0), m_vbo(0),
    m_initialized(false) {
}

Flag::~Flag() {
    cleanup();
}

void Flag::initialize(int width, int height, float spacing, const glm::vec3& anchorPos) {
    m_width = width;
    m_height = height;
    m_spacing = spacing;

    createMesh(width, height, spacing, anchorPos);

    // Create OpenGL buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    updateVertexBuffer();

    m_initialized = true;
    std::cout << "Flag initialized: " << m_width << "x" << m_height
              << " grid, " << m_particles.size() << " particles, "
              << m_springs.size() << " springs" << std::endl;
}

void Flag::createMesh(int width, int height, float spacing, const glm::vec3& anchorPos) {
    m_particles.clear();
    m_springs.clear();

    // Create particles in a grid
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Particle p;
            p.position = anchorPos + glm::vec3(x * spacing, 0.0f, -y * spacing);
            p.oldPosition = p.position;
            p.acceleration = glm::vec3(0.0f);
            p.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            p.mass = 1.0f;

            // Fix the left edge (pole attachment)
            p.fixed = (x == 0);

            m_particles.push_back(p);
        }
    }

    // Create structural springs (horizontal and vertical)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Horizontal spring
            if (x < width - 1) {
                addSpring(getIndex(x, y), getIndex(x + 1, y), m_springStiffness);
            }

            // Vertical spring
            if (y < height - 1) {
                addSpring(getIndex(x, y), getIndex(x, y + 1), m_springStiffness);
            }
        }
    }

    // Create shear springs (diagonals)
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            addSpring(getIndex(x, y), getIndex(x + 1, y + 1), m_springStiffness * 0.5f);
            addSpring(getIndex(x + 1, y), getIndex(x, y + 1), m_springStiffness * 0.5f);
        }
    }

    // Create bending springs (skip one vertex)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 2; x++) {
            addSpring(getIndex(x, y), getIndex(x + 2, y), m_springStiffness * 0.25f);
        }
    }

    for (int y = 0; y < height - 2; y++) {
        for (int x = 0; x < width; x++) {
            addSpring(getIndex(x, y), getIndex(x, y + 2), m_springStiffness * 0.25f);
        }
    }
}

void Flag::addSpring(int p1, int p2, float stiffness) {
    Spring s;
    s.p1Index = p1;
    s.p2Index = p2;
    s.restLength = glm::length(m_particles[p1].position - m_particles[p2].position);
    s.stiffness = stiffness;
    s.damping = m_springDamping;
    m_springs.push_back(s);
}

void Flag::update(float deltaTime) {
    const int numIterations = 5;
    const float dt = deltaTime;

    // Track accumulated time for wind variation
    static float windTime = 0.0f;
    windTime += deltaTime;

    // Apply forces with time-varying wind
    for (Particle& p : m_particles) {
        if (p.fixed) continue;

        // Gravity
        p.acceleration += m_gravity;

        // Time-varying wind
        glm::vec3 wind = m_windForce;
        wind.x += sin(windTime * 2.0f) * 1.0f;
        wind.z += cos(windTime * 1.5f) * 0.5f;

        // Wind strength increases away from pole
        int x = (&p - &m_particles[0]) % m_width;
        float windStrength = (float)x / (m_width - 1);
        p.acceleration += wind * windStrength;

        // Air resistance/damping
        glm::vec3 velocity = p.position - p.oldPosition;
        p.acceleration -= velocity * 0.1f;
    }

    // Verlet integration
    for (Particle& p : m_particles) {
        if (p.fixed) continue;

        glm::vec3 temp = p.position;
        glm::vec3 velocity = (p.position - p.oldPosition);

        // Verlet: x(t+dt) = x(t) + v(t)*dt + a(t)*dt^2
        p.position = p.position + velocity + p.acceleration * (dt * dt);
        p.oldPosition = temp;

        // Reset acceleration
        p.acceleration = glm::vec3(0.0f);
    }

    // Satisfy constraints multiple times
    for (int i = 0; i < numIterations; i++) {
        satisfyConstraints();
    }

    // Update normals and vertex buffer
    updateNormals();
    updateVertexBuffer();
}

void Flag::satisfyConstraints() {
    // Spring constraints
    for (const Spring& s : m_springs) {
        Particle& p1 = m_particles[s.p1Index];
        Particle& p2 = m_particles[s.p2Index];

        if (p1.fixed && p2.fixed) continue;

        glm::vec3 delta = p2.position - p1.position;
        float currentLength = glm::length(delta);

        if (currentLength < 0.0001f) continue;

        float difference = (currentLength - s.restLength) / currentLength;

        // Apply maximum stretch constraint
        if (currentLength > s.restLength * m_maxStretch) {
            difference = (currentLength - s.restLength * m_maxStretch) / currentLength;
        }

        glm::vec3 correction = delta * 0.5f * difference;

        if (!p1.fixed) p1.position += correction;
        if (!p2.fixed) p2.position -= correction;
    }
}

void Flag::updateNormals() {
    // Reset normals
    for (Particle& p : m_particles) {
        p.normal = glm::vec3(0.0f);
    }

    // Calculate face normals and accumulate
    for (int y = 0; y < m_height - 1; y++) {
        for (int x = 0; x < m_width - 1; x++) {
            int i0 = getIndex(x, y);
            int i1 = getIndex(x + 1, y);
            int i2 = getIndex(x, y + 1);
            int i3 = getIndex(x + 1, y + 1);

            // Triangle 1
            glm::vec3 v1 = m_particles[i1].position - m_particles[i0].position;
            glm::vec3 v2 = m_particles[i2].position - m_particles[i0].position;
            glm::vec3 normal1 = glm::normalize(glm::cross(v1, v2));

            m_particles[i0].normal += normal1;
            m_particles[i1].normal += normal1;
            m_particles[i2].normal += normal1;

            // Triangle 2
            v1 = m_particles[i3].position - m_particles[i1].position;
            v2 = m_particles[i2].position - m_particles[i1].position;
            glm::vec3 normal2 = glm::normalize(glm::cross(v1, v2));

            m_particles[i1].normal += normal2;
            m_particles[i2].normal += normal2;
            m_particles[i3].normal += normal2;
        }
    }

    // Normalize
    for (Particle& p : m_particles) {
        if (glm::length(p.normal) > 0.0001f) {
            p.normal = glm::normalize(p.normal);
        }
    }
}

void Flag::updateVertexBuffer() {
    m_vertexData.clear();

    // Create triangles for rendering
    for (int y = 0; y < m_height - 1; y++) {
        for (int x = 0; x < m_width - 1; x++) {
            int i0 = getIndex(x, y);
            int i1 = getIndex(x + 1, y);
            int i2 = getIndex(x, y + 1);
            int i3 = getIndex(x + 1, y + 1);

            // Triangle 1
            m_vertexData.push_back(m_particles[i0].position.x);
            m_vertexData.push_back(m_particles[i0].position.y);
            m_vertexData.push_back(m_particles[i0].position.z);
            m_vertexData.push_back(m_particles[i0].normal.x);
            m_vertexData.push_back(m_particles[i0].normal.y);
            m_vertexData.push_back(m_particles[i0].normal.z);

            m_vertexData.push_back(m_particles[i1].position.x);
            m_vertexData.push_back(m_particles[i1].position.y);
            m_vertexData.push_back(m_particles[i1].position.z);
            m_vertexData.push_back(m_particles[i1].normal.x);
            m_vertexData.push_back(m_particles[i1].normal.y);
            m_vertexData.push_back(m_particles[i1].normal.z);

            m_vertexData.push_back(m_particles[i2].position.x);
            m_vertexData.push_back(m_particles[i2].position.y);
            m_vertexData.push_back(m_particles[i2].position.z);
            m_vertexData.push_back(m_particles[i2].normal.x);
            m_vertexData.push_back(m_particles[i2].normal.y);
            m_vertexData.push_back(m_particles[i2].normal.z);

            // Triangle 2
            m_vertexData.push_back(m_particles[i1].position.x);
            m_vertexData.push_back(m_particles[i1].position.y);
            m_vertexData.push_back(m_particles[i1].position.z);
            m_vertexData.push_back(m_particles[i1].normal.x);
            m_vertexData.push_back(m_particles[i1].normal.y);
            m_vertexData.push_back(m_particles[i1].normal.z);

            m_vertexData.push_back(m_particles[i3].position.x);
            m_vertexData.push_back(m_particles[i3].position.y);
            m_vertexData.push_back(m_particles[i3].position.z);
            m_vertexData.push_back(m_particles[i3].normal.x);
            m_vertexData.push_back(m_particles[i3].normal.y);
            m_vertexData.push_back(m_particles[i3].normal.z);

            m_vertexData.push_back(m_particles[i2].position.x);
            m_vertexData.push_back(m_particles[i2].position.y);
            m_vertexData.push_back(m_particles[i2].position.z);
            m_vertexData.push_back(m_particles[i2].normal.x);
            m_vertexData.push_back(m_particles[i2].normal.y);
            m_vertexData.push_back(m_particles[i2].normal.z);
        }
    }

    // Upload to GPU
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float),
                 m_vertexData.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Flag::render() {
    if (!m_initialized) return;

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, (m_width - 1) * (m_height - 1) * 6);
    glBindVertexArray(0);
}

void Flag::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    m_initialized = false;
}
