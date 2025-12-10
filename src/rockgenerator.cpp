#include "rockgenerator.h"
#include <cmath>
#include <iostream>
#include <map>
#include <tuple>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RockGenerator::RockGenerator()
    : m_vao(0), m_vbo(0), m_vertexCount(0), m_hasRock(false), m_initialized(false) {
}

RockGenerator::~RockGenerator() {
    cleanup();
}

void RockGenerator::initialize() {
    if (m_initialized) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    m_initialized = true;
    std::cout << "Rock generator initialized" << std::endl;
}

void RockGenerator::generateRock(int detail) {
    if (!m_initialized) return;

    generateIcosphere(detail);

    smoothNormalsAfterSubdivision();

    calculateTangents();

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float),
                 m_vertexData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                          (void*)(6 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_hasRock = true;
    std::cout << "Rock generated: " << m_vertexCount << " vertices" << std::endl;
}

void RockGenerator::generateIcosphere(int subdivisions) {
    m_vertexData.clear();

    float t = (1.0f + sqrt(5.0f)) / 2.0f;

    std::vector<glm::vec3> vertices = {
        {-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
        {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
        {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}
    };

    for (auto& v : vertices) {
        v = glm::normalize(v);
    }

    std::vector<glm::ivec3> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    for (int i = 0; i < subdivisions; i++) {
        std::vector<glm::ivec3> newFaces;
        std::map<std::pair<int,int>, int> midpointCache;

        auto getMidpoint = [&](int i1, int i2) -> int {
            auto key = std::minmax(i1, i2);
            auto it = midpointCache.find(key);
            if (it != midpointCache.end()) {
                return it->second;
            }

            glm::vec3 mid = glm::normalize((vertices[i1] + vertices[i2]) * 0.5f);
            int idx = vertices.size();
            vertices.push_back(mid);
            midpointCache[key] = idx;
            return idx;
        };

        for (const auto& face : faces) {
            int a = getMidpoint(face.x, face.y);
            int b = getMidpoint(face.y, face.z);
            int c = getMidpoint(face.z, face.x);

            newFaces.push_back({face.x, a, c});
            newFaces.push_back({face.y, b, a});
            newFaces.push_back({face.z, c, b});
            newFaces.push_back({a, b, c});
        }

        faces = newFaces;
    }

    for (const auto& face : faces) {
        glm::vec3 v0 = vertices[face.x];
        glm::vec3 v1 = vertices[face.y];
        glm::vec3 v2 = vertices[face.z];

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        glm::vec3 tangent(1, 0, 0);

        m_vertexData.push_back(v0.x); m_vertexData.push_back(v0.y); m_vertexData.push_back(v0.z);
        m_vertexData.push_back(normal.x); m_vertexData.push_back(normal.y); m_vertexData.push_back(normal.z);
        m_vertexData.push_back(tangent.x); m_vertexData.push_back(tangent.y); m_vertexData.push_back(tangent.z);

        m_vertexData.push_back(v1.x); m_vertexData.push_back(v1.y); m_vertexData.push_back(v1.z);
        m_vertexData.push_back(normal.x); m_vertexData.push_back(normal.y); m_vertexData.push_back(normal.z);
        m_vertexData.push_back(tangent.x); m_vertexData.push_back(tangent.y); m_vertexData.push_back(tangent.z);

        m_vertexData.push_back(v2.x); m_vertexData.push_back(v2.y); m_vertexData.push_back(v2.z);
        m_vertexData.push_back(normal.x); m_vertexData.push_back(normal.y); m_vertexData.push_back(normal.z);
        m_vertexData.push_back(tangent.x); m_vertexData.push_back(tangent.y); m_vertexData.push_back(tangent.z);
    }

    m_vertexCount = m_vertexData.size() / 9;
}

void RockGenerator::perturbVertices() {
    for (size_t i = 0; i < m_vertexData.size(); i += 9) {
        glm::vec3 pos(m_vertexData[i], m_vertexData[i+1], m_vertexData[i+2]);
        glm::vec3 normal(m_vertexData[i+3], m_vertexData[i+4], m_vertexData[i+5]);

        float noise = sin(pos.x * 5.0f) * cos(pos.y * 5.0f) * sin(pos.z * 5.0f);
        noise += sin(pos.x * 10.0f) * cos(pos.y * 10.0f) * 0.5f;
        noise *= 0.15f;

        pos += normal * noise;

        m_vertexData[i] = pos.x;
        m_vertexData[i+1] = pos.y;
        m_vertexData[i+2] = pos.z;
    }

    for (size_t i = 0; i < m_vertexData.size(); i += 27) {
        glm::vec3 v0(m_vertexData[i], m_vertexData[i+1], m_vertexData[i+2]);
        glm::vec3 v1(m_vertexData[i+9], m_vertexData[i+10], m_vertexData[i+11]);
        glm::vec3 v2(m_vertexData[i+18], m_vertexData[i+19], m_vertexData[i+20]);

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        for (int j = 0; j < 3; j++) {
            m_vertexData[i + j*9 + 3] = normal.x;
            m_vertexData[i + j*9 + 4] = normal.y;
            m_vertexData[i + j*9 + 5] = normal.z;
        }
    }
}

void RockGenerator::calculateTangents() {
    for (size_t i = 0; i < m_vertexData.size(); i += 27) {
        glm::vec3 v0(m_vertexData[i], m_vertexData[i+1], m_vertexData[i+2]);
        glm::vec3 v1(m_vertexData[i+9], m_vertexData[i+10], m_vertexData[i+11]);
        glm::vec3 v2(m_vertexData[i+18], m_vertexData[i+19], m_vertexData[i+20]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        glm::vec3 normal(m_vertexData[i+3], m_vertexData[i+4], m_vertexData[i+5]);

        glm::vec3 tangent;
        if (abs(normal.y) < 0.9f) {
            tangent = glm::normalize(glm::cross(normal, glm::vec3(0, 1, 0)));
        } else {
            tangent = glm::normalize(glm::cross(normal, glm::vec3(1, 0, 0)));
        }

        for (int j = 0; j < 3; j++) {
            m_vertexData[i + j*9 + 6] = tangent.x;
            m_vertexData[i + j*9 + 7] = tangent.y;
            m_vertexData[i + j*9 + 8] = tangent.z;
        }
    }
}

void RockGenerator::draw() {
    if (!m_hasRock || m_vertexCount == 0) return;

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}

void RockGenerator::cleanup() {
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

void RockGenerator::calculateSmoothNormals() {
    std::map<std::tuple<int,int,int>, std::vector<size_t>> positionMap;

    for (size_t i = 0; i < m_vertexData.size(); i += 9) {
        float x = m_vertexData[i];
        float y = m_vertexData[i+1];
        float z = m_vertexData[i+2];

        int ix = (int)std::round(x * 10000.0f);
        int iy = (int)std::round(y * 10000.0f);
        int iz = (int)std::round(z * 10000.0f);

        auto key = std::make_tuple(ix, iy, iz);
        positionMap[key].push_back(i / 9);
    }

    std::vector<glm::vec3> smoothNormals(m_vertexData.size() / 9, glm::vec3(0.0f));
    std::vector<int> normalCounts(m_vertexData.size() / 9, 0);

    for (size_t i = 0; i < m_vertexData.size(); i += 27) {
        glm::vec3 v0(m_vertexData[i], m_vertexData[i+1], m_vertexData[i+2]);
        glm::vec3 v1(m_vertexData[i+9], m_vertexData[i+10], m_vertexData[i+11]);
        glm::vec3 v2(m_vertexData[i+18], m_vertexData[i+19], m_vertexData[i+20]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        size_t idx0 = i / 9;
        size_t idx1 = (i + 9) / 9;
        size_t idx2 = (i + 18) / 9;

        smoothNormals[idx0] += faceNormal;
        smoothNormals[idx1] += faceNormal;
        smoothNormals[idx2] += faceNormal;

        normalCounts[idx0]++;
        normalCounts[idx1]++;
        normalCounts[idx2]++;
    }

    for (const auto& pair : positionMap) {
        const std::vector<size_t>& indices = pair.second;

        glm::vec3 avgNormal(0.0f);
        for (size_t vertexIdx : indices) {
            avgNormal += smoothNormals[vertexIdx];
        }
        avgNormal = glm::normalize(avgNormal);

        for (size_t vertexIdx : indices) {
            size_t dataIdx = vertexIdx * 9;
            m_vertexData[dataIdx + 3] = avgNormal.x;
            m_vertexData[dataIdx + 4] = avgNormal.y;
            m_vertexData[dataIdx + 5] = avgNormal.z;
        }
    }
}

void RockGenerator::smoothNormalsAfterSubdivision() {
    for (size_t i = 0; i < m_vertexData.size(); i += 9) {
        glm::vec3 pos(m_vertexData[i], m_vertexData[i+1], m_vertexData[i+2]);
        glm::vec3 normal = glm::normalize(pos);

        m_vertexData[i+3] = normal.x;
        m_vertexData[i+4] = normal.y;
        m_vertexData[i+5] = normal.z;
    }
}

std::vector<float> RockGenerator::getVertexDataSimple() const {
    std::vector<float> simpleData;
    simpleData.reserve((m_vertexData.size() / 9) * 6);

    for (size_t i = 0; i < m_vertexData.size(); i += 9) {
        simpleData.push_back(m_vertexData[i]);
        simpleData.push_back(m_vertexData[i+1]);
        simpleData.push_back(m_vertexData[i+2]);

        simpleData.push_back(m_vertexData[i+3]);
        simpleData.push_back(m_vertexData[i+4]);
        simpleData.push_back(m_vertexData[i+5]);
    }

    return simpleData;
}
