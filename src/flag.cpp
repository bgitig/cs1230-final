#include "flag.h"
#include "terrain.h"
#include <iostream>
#include <cmath>
#include <QImage>

Flag::Flag()
    : m_width(0), m_height(0), m_spacing(0.0f),
    m_windForce(10.0f, 0.0f, 0.0f), //FOR WIND CONTROL
    m_gravity(0.0f, 0.0f, -9.8f),
    m_springStiffness(50.0f),
    m_springDamping(0.25f),
    m_terrain(nullptr), m_hasTerrainCollision(false),
    m_maxStretch(1.1f),
    m_vao(0), m_vbo(0),
    m_poleVao(0), m_poleVbo(0),
    m_poleVertexCount(0),
    m_initialized(false),
    m_textureID(0), m_useTexture(false){
}
bool Flag::loadTexture(const std::string& filepath) {
    QImage image(QString::fromStdString(filepath));
    if (image.isNull()) {
        std::cerr << "Failed to load flag texture: " << filepath << std::endl;
        return false;
    }

    // Convert to OpenGL format
    image = image.convertToFormat(QImage::Format_RGBA8888);
    image = image.mirrored();  // Flip vertically for OpenGL

    // Generate texture
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_useTexture = true;
    std::cout << "Loaded flag texture: " << filepath << std::endl;
    return true;
}

Flag::~Flag() {
    cleanup();
}

void Flag::initialize(int width, int height, float spacing, const glm::vec3& anchorPos) {
    m_width = width;
    m_height = height;
    m_spacing = spacing;

    createMesh(width, height, spacing, anchorPos);

    // Create pole geometry - MASSIVE for visibility
    float poleHeight = (height - 1) * spacing + 0.05f;
    float poleRadius = spacing * 10.0f;  // 10x spacing for visibility!
    createPole(poleHeight, poleRadius);

    // Create OpenGL buffers for flag
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // Create OpenGL buffers for pole
    glGenVertexArrays(1, &m_poleVao);
    glGenBuffers(1, &m_poleVbo);

    updateVertexBuffer();

    m_initialized = true;
    std::cout << "Flag initialized with pole (radius=" << poleRadius << ")" << std::endl;
}

void Flag::createMesh(int width, int height, float spacing, const glm::vec3& anchorPos) {
    m_particles.clear();
    m_springs.clear();

    // Create particles in a grid
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Particle p;
            // Flag extends HORIZONTALLY: X to the right, Y is depth, Z is height
            float zPosition = anchorPos.z - (y * spacing);  // Top to bottom
            p.position = glm::vec3(anchorPos.x + x * spacing, anchorPos.y, zPosition);

            p.oldPosition = p.position;
            p.acceleration = glm::vec3(0.0f);
            p.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            p.mass = 1.0f;

            // Fix the left edge (pole attachment at X=0)
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

    static float windTime = 0.0f;
    windTime += deltaTime;

    for (Particle& p : m_particles) {
        if (p.fixed) continue;

        // LESS gravity - flag should flutter, not hang heavy
        glm::vec3 gravity(0.0f, 0.0f, -2.0f);  // REDUCED from -9.8
        p.acceleration += gravity;

        // Wind in +Y direction (blowing AWAY from viewer)
        glm::vec3 wind(0.0f, 8.0f, 0.0f);  // Changed from X to Y direction
        wind.y += sin(windTime * 2.0f) * 4.0f;
        wind.x += cos(windTime * 1.5f) * 2.0f;  // Some side-to-side

        // Wind strength increases away from pole
        int x = (&p - &m_particles[0]) % m_width;
        float windStrength = (float)x / (m_width - 1);
        p.acceleration += wind * windStrength;

        // Air resistance
        glm::vec3 velocity = p.position - p.oldPosition;
        p.acceleration -= velocity * 0.1f;  // Increased damping
    }

    /*
    // Track accumulated time for wind variation
    static float windTime = 0.0f;
    windTime += deltaTime;

    // Apply forces with time-varying wind
    for (Particle& p : m_particles) {
        if (p.fixed) continue;

        // Gravity
        p.acceleration += m_gravity;

        //IF WE WANT TO PLAY WITH THE WIND
        glm::vec3 wind = m_windForce;
        wind.x += sin(windTime * 2.0f) * 3.0f;
        wind.z += cos(windTime * 1.5f) * 2.0f;
        wind.x += sin(windTime * 5.0f) * 1.5f;

        // Wind strength increases away from pole
        int x = (&p - &m_particles[0]) % m_width;
        float windStrength = (float)x / (m_width - 1);
        p.acceleration += wind * windStrength;

        // Air resistance/damping
        glm::vec3 velocity = p.position - p.oldPosition;
        p.acceleration -= velocity * 0.05f;
    }
    */

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
        if (m_hasTerrainCollision) {
            handleTerrainCollision();
        }
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

    // Create triangles with UV coordinates (now 8 floats: pos(3) + normal(3) + uv(2))
    for (int y = 0; y < m_height - 1; y++) {
        for (int x = 0; x < m_width - 1; x++) {
            int i0 = getIndex(x, y);
            int i1 = getIndex(x + 1, y);
            int i2 = getIndex(x, y + 1);
            int i3 = getIndex(x + 1, y + 1);

            // Calculate UV coordinates
            float u0 = (float)x / (m_width - 1);
            float u1 = (float)(x + 1) / (m_width - 1);
            float v0 = (float)y / (m_height - 1);
            float v1 = (float)(y + 1) / (m_height - 1);

            // Triangle 1: i0, i1, i2
            // Vertex i0
            m_vertexData.push_back(m_particles[i0].position.x);
            m_vertexData.push_back(m_particles[i0].position.y);
            m_vertexData.push_back(m_particles[i0].position.z);
            m_vertexData.push_back(m_particles[i0].normal.x);
            m_vertexData.push_back(m_particles[i0].normal.y);
            m_vertexData.push_back(m_particles[i0].normal.z);
            m_vertexData.push_back(u0);  // NEW: U coordinate
            m_vertexData.push_back(v0);  // NEW: V coordinate

            // Vertex i1
            m_vertexData.push_back(m_particles[i1].position.x);
            m_vertexData.push_back(m_particles[i1].position.y);
            m_vertexData.push_back(m_particles[i1].position.z);
            m_vertexData.push_back(m_particles[i1].normal.x);
            m_vertexData.push_back(m_particles[i1].normal.y);
            m_vertexData.push_back(m_particles[i1].normal.z);
            m_vertexData.push_back(u1);
            m_vertexData.push_back(v0);

            // Vertex i2
            m_vertexData.push_back(m_particles[i2].position.x);
            m_vertexData.push_back(m_particles[i2].position.y);
            m_vertexData.push_back(m_particles[i2].position.z);
            m_vertexData.push_back(m_particles[i2].normal.x);
            m_vertexData.push_back(m_particles[i2].normal.y);
            m_vertexData.push_back(m_particles[i2].normal.z);
            m_vertexData.push_back(u0);
            m_vertexData.push_back(v1);

            // Triangle 2: i1, i3, i2
            // Vertex i1
            m_vertexData.push_back(m_particles[i1].position.x);
            m_vertexData.push_back(m_particles[i1].position.y);
            m_vertexData.push_back(m_particles[i1].position.z);
            m_vertexData.push_back(m_particles[i1].normal.x);
            m_vertexData.push_back(m_particles[i1].normal.y);
            m_vertexData.push_back(m_particles[i1].normal.z);
            m_vertexData.push_back(u1);
            m_vertexData.push_back(v0);

            // Vertex i3
            m_vertexData.push_back(m_particles[i3].position.x);
            m_vertexData.push_back(m_particles[i3].position.y);
            m_vertexData.push_back(m_particles[i3].position.z);
            m_vertexData.push_back(m_particles[i3].normal.x);
            m_vertexData.push_back(m_particles[i3].normal.y);
            m_vertexData.push_back(m_particles[i3].normal.z);
            m_vertexData.push_back(u1);
            m_vertexData.push_back(v1);

            // Vertex i2
            m_vertexData.push_back(m_particles[i2].position.x);
            m_vertexData.push_back(m_particles[i2].position.y);
            m_vertexData.push_back(m_particles[i2].position.z);
            m_vertexData.push_back(m_particles[i2].normal.x);
            m_vertexData.push_back(m_particles[i2].normal.y);
            m_vertexData.push_back(m_particles[i2].normal.z);
            m_vertexData.push_back(u0);
            m_vertexData.push_back(v1);
        }
    }

    // Upload to GPU with new stride (8 floats instead of 6)
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float),
                 m_vertexData.data(), GL_DYNAMIC_DRAW);

    // Position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    // UV coordinates (location 2) - NEW
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(6 * sizeof(float)));

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
    if (m_poleVao != 0) {
        glDeleteVertexArrays(1, &m_poleVao);
        m_poleVao = 0;
    }
    if (m_poleVbo != 0) {
        glDeleteBuffers(1, &m_poleVbo);
        m_poleVbo = 0;
    }
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    m_initialized = false;
}

void Flag::createPole(float poleHeight, float poleRadius) {
    m_poleVertexData.clear();

    // Vertical pole on the left side
    float radius = poleRadius * 0.3f;

    float x0 = -radius;
    float x1 = radius;
    float y0 = -radius;
    float y1 = radius;
    float z0 = 0.0f;  // Bottom (ground level)
    float z1 = poleHeight;   // Extend below flag

    // Helper to add a quad (2 triangles)
    auto addQuad = [&](float ax, float ay, float az,
                       float bx, float by, float bz,
                       float cx, float cy, float cz,
                       float dx, float dy, float dz,
                       glm::vec3 normal) {
        // Triangle 1
        m_poleVertexData.push_back(ax); m_poleVertexData.push_back(ay); m_poleVertexData.push_back(az);
        m_poleVertexData.push_back(normal.x); m_poleVertexData.push_back(normal.y); m_poleVertexData.push_back(normal.z);

        m_poleVertexData.push_back(bx); m_poleVertexData.push_back(by); m_poleVertexData.push_back(bz);
        m_poleVertexData.push_back(normal.x); m_poleVertexData.push_back(normal.y); m_poleVertexData.push_back(normal.z);

        m_poleVertexData.push_back(cx); m_poleVertexData.push_back(cy); m_poleVertexData.push_back(cz);
        m_poleVertexData.push_back(normal.x); m_poleVertexData.push_back(normal.y); m_poleVertexData.push_back(normal.z);

        // Triangle 2
        m_poleVertexData.push_back(ax); m_poleVertexData.push_back(ay); m_poleVertexData.push_back(az);
        m_poleVertexData.push_back(normal.x); m_poleVertexData.push_back(normal.y); m_poleVertexData.push_back(normal.z);

        m_poleVertexData.push_back(cx); m_poleVertexData.push_back(cy); m_poleVertexData.push_back(cz);
        m_poleVertexData.push_back(normal.x); m_poleVertexData.push_back(normal.y); m_poleVertexData.push_back(normal.z);

        m_poleVertexData.push_back(dx); m_poleVertexData.push_back(dy); m_poleVertexData.push_back(dz);
        m_poleVertexData.push_back(normal.x); m_poleVertexData.push_back(normal.y); m_poleVertexData.push_back(normal.z);
    };

    // Front face (+Y)
    addQuad(x0, y1, z0,  x1, y1, z0,  x1, y1, z1,  x0, y1, z1,  glm::vec3(0, 1, 0));

    // Back face (-Y)
    addQuad(x1, y0, z0,  x0, y0, z0,  x0, y0, z1,  x1, y0, z1,  glm::vec3(0, -1, 0));

    // Right face (+X) - this faces the flag
    addQuad(x1, y1, z0,  x1, y0, z0,  x1, y0, z1,  x1, y1, z1,  glm::vec3(1, 0, 0));

    // Left face (-X)
    addQuad(x0, y0, z0,  x0, y1, z0,  x0, y1, z1,  x0, y0, z1,  glm::vec3(-1, 0, 0));

    // Top face (+Z)
    addQuad(x0, y0, z0,  x1, y0, z0,  x1, y1, z0,  x0, y1, z0,  glm::vec3(0, 0, 1));

    // Bottom face (-Z)
    addQuad(x0, y1, z1,  x1, y1, z1,  x1, y0, z1,  x0, y0, z1,  glm::vec3(0, 0, -1));

    m_poleVertexCount = m_poleVertexData.size() / 6;

    std::cout << "POLE: " << m_poleVertexCount << " verts, x:[" << x0 << " to " << x1
              << "], z:[" << z1 << " to " << z0 << "]" << std::endl;

    // Upload to GPU
    glBindVertexArray(m_poleVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_poleVbo);
    glBufferData(GL_ARRAY_BUFFER, m_poleVertexData.size() * sizeof(float),
                 m_poleVertexData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Flag::renderPole() {
    if (!m_initialized || m_poleVertexCount == 0) {
        std::cout << "Cannot render pole: initialized=" << m_initialized
                  << ", vertexCount=" << m_poleVertexCount << std::endl;
        return;
    }

    glBindVertexArray(m_poleVao);
    glDrawArrays(GL_TRIANGLES, 0, m_poleVertexCount);
    glBindVertexArray(0);
}


//COLLISION

void Flag::setTerrain(Terrain* terrain, const glm::mat4& worldMatrix) {
    m_terrain = terrain;
    m_worldMatrix = worldMatrix;
    m_hasTerrainCollision = true;
}

void Flag::handleTerrainCollision() {
    if (!m_terrain || !m_hasTerrainCollision) return;

    for (Particle& p : m_particles) {
        if (p.fixed) continue;

        // Transform particle to terrain space (inverse of world matrix)
        glm::mat4 invWorld = glm::inverse(m_worldMatrix);
        glm::vec4 terrainSpacePos = invWorld * glm::vec4(p.position, 1.0f);

        // Get terrain coordinates (0-1 range)
        float terrainX = terrainSpacePos.x;
        float terrainY = terrainSpacePos.y;

        // Check if within terrain bounds
        if (terrainX < 0.0f || terrainX > 1.0f || terrainY < 0.0f || terrainY > 1.0f) {
            continue;  // Outside terrain bounds
        }

        // Get terrain height at this position
        float terrainHeight = m_terrain->getHeight(terrainX, terrainY);

        // Transform terrain height to world space
        glm::vec4 terrainWorldPoint = m_worldMatrix * glm::vec4(terrainX, terrainY, terrainHeight, 1.0f);
        float terrainWorldZ = terrainWorldPoint.z;

        // Collision detection: if particle is below terrain
        float offset = 0.01f;  // Small offset to prevent sinking
        if (p.position.z < terrainWorldZ + offset) {
            // Push particle above terrain
            p.position.z = terrainWorldZ + offset;

            // Dampen velocity (simulate friction)
            glm::vec3 velocity = p.position - p.oldPosition;
            velocity *= 0.3f;  // Reduce velocity by 70%
            p.oldPosition = p.position - velocity;
        }
    }
}

