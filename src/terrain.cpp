#include "terrain.h"

#include <cmath>
#include "glm/glm.hpp"
#include <iostream>
#include <algorithm>
#include <set>

// Constructor
Terrain::Terrain()
{
    m_wireshade = false;

    // Define resolution of terrain generation
    m_resolution = 100;
    m_tilesPerSide = 10;  // 10x10 grid of tiles
    m_tileResolution = m_resolution / m_tilesPerSide;  // 10 vertices per tile

    // Generate random vector lookup table
    m_lookupSize = 1024;
    m_randVecLookup.reserve(m_lookupSize);

    // Initialize random number generator
    std::srand(1230);

    // Populate random vector lookup table
    for (int i = 0; i < m_lookupSize; i++)
    {
        m_randVecLookup.push_back(glm::vec2(std::rand() * 2.0 / RAND_MAX - 1.0,
                                            std::rand() * 2.0 / RAND_MAX - 1.0));
    }
}

// Destructor
Terrain::~Terrain()
{
    m_randVecLookup.clear();
    m_displacements.clear();
}

// add a dip at the specified location
void Terrain::divot(float x, float y, float depth, float radius) {
    m_displacements.push_back(glm::vec4(x, y, depth, radius));
}

// calculate which tile contains the clicked coordinates
void Terrain::getTileCoordinates(float x, float y, int& tileX, int& tileY) {
    tileX = std::min((int)(x * m_tilesPerSide), m_tilesPerSide - 1);
    tileY = std::min((int)(y * m_tilesPerSide), m_tilesPerSide - 1);
    tileX = std::max(0, tileX);
    tileY = std::max(0, tileY);
}

// get all tiles affected by a crater at (x, y) with given radius
std::vector<int> Terrain::getAffectedTiles(float x, float y, float radius) {
    std::set<int> affectedTilesSet;

    // bounding box
    float minX = x - radius;
    float maxX = x + radius;
    float minY = y - radius;
    float maxY = y + radius;

    // convert to tile coordinates
    int minTileX, minTileY, maxTileX, maxTileY;
    getTileCoordinates(minX, minY, minTileX, minTileY);
    getTileCoordinates(maxX, maxY, maxTileX, maxTileY);

    minTileX = std::max(0, minTileX - 1);
    minTileY = std::max(0, minTileY - 1);
    maxTileX = std::min(m_tilesPerSide - 1, maxTileX + 1);
    maxTileY = std::min(m_tilesPerSide - 1, maxTileY + 1);

    for (int tx = minTileX; tx <= maxTileX; tx++) {
        for (int ty = minTileY; ty <= maxTileY; ty++) {
            affectedTilesSet.insert(ty * m_tilesPerSide + tx);
        }
    }

    std::vector<int> affectedTiles(affectedTilesSet.begin(), affectedTilesSet.end());
    return affectedTiles;
}

float Terrain::getHeightModification(float x, float y) {
    float totalModification = 0.0f;

    for (const auto& crater : m_displacements) {
        float craterX = crater.x;
        float craterY = crater.y;
        float depth = crater.z;
        float radius = crater.w;

        // calculate distance from this point to crater center
        float dx = x - craterX;
        float dy = y - craterY;
        float distance = std::sqrt(dx * dx + dy * dy);

        // smooth step
        if (distance < radius) {
            float t = distance / radius;
            float falloff = 1.0f - (3.0f * t * t - 2.0f * t * t * t);
            totalModification -= depth * falloff;
        }
    }

    return totalModification;
}

// Helper for generateTerrain()
void addPointToVector(glm::vec3 point, std::vector<float>& vector) {
    vector.push_back(point.x);
    vector.push_back(point.y);
    vector.push_back(point.z);
}

// generate a single tile
std::vector<float> Terrain::generateTile(int tileX, int tileY) {
    std::vector<float> verts;
    verts.reserve(m_tileResolution * m_tileResolution * 54);

    int startX = tileX * m_tileResolution;
    int startY = tileY * m_tileResolution;

    for(int x = startX; x < startX + m_tileResolution; x++) {
        for(int y = startY; y < startY + m_tileResolution; y++) {
            int x1 = x;
            int y1 = y;
            int x2 = x + 1;
            int y2 = y + 1;

            glm::vec3 p1 = getPosition(x1, y1);
            glm::vec3 p2 = getPosition(x2, y1);
            glm::vec3 p3 = getPosition(x2, y2);
            glm::vec3 p4 = getPosition(x1, y2);

            glm::vec3 n1 = getNormal(x1, y1);
            glm::vec3 n2 = getNormal(x2, y1);
            glm::vec3 n3 = getNormal(x2, y2);
            glm::vec3 n4 = getNormal(x1, y2);

            // triangle 1
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p2, verts);
            addPointToVector(n2, verts);
            addPointToVector(getColor(n2, p2), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            // triangle 2
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            addPointToVector(p4, verts);
            addPointToVector(n4, verts);
            addPointToVector(getColor(n4, p4), verts);
        }
    }
    return verts;
}

// tile-specific vbo copy
void Terrain::updateTile(int tileX, int tileY, std::vector<float>& allVerts) {
    std::vector<float> tileVerts = generateTile(tileX, tileY);

    int tileIndex = tileY * m_tilesPerSide + tileX;
    int vertsPerTile = m_tileResolution * m_tileResolution * 54;
    int startIndex = tileIndex * vertsPerTile;

    // copy the tile vertices into the main buffer
    for (size_t i = 0; i < tileVerts.size(); i++) {
        allVerts[startIndex + i] = tileVerts[i];
    }

}

// Generates the geometry of the entire terrain (all tiles)
std::vector<float> Terrain::generateTerrain() {
    std::vector<float> verts;
    int totalVerts = m_resolution * m_resolution * 54;
    verts.reserve(totalVerts);

    // Generate all tiles
    for (int tileY = 0; tileY < m_tilesPerSide; tileY++) {
        for (int tileX = 0; tileX < m_tilesPerSide; tileX++) {
            std::vector<float> tileVerts = generateTile(tileX, tileY);
            verts.insert(verts.end(), tileVerts.begin(), tileVerts.end());
        }
    }

    return verts;
}

// Samples the (infinite) random vector grid at (row, col)
glm::vec2 Terrain::sampleRandomVector(int row, int col)
{
    std::hash<int> intHash;
    int index = intHash(row * 41 + col * 43) % m_lookupSize;
    return m_randVecLookup.at(index);
}

// Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
// Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
glm::vec3 Terrain::getPosition(int row, int col) {
    float x = 1.0 * row / m_resolution;
    float y = 1.0 * col / m_resolution;
    float z = getHeight(x, y);
    return glm::vec3(x, y, z);
}

// Helper for computePerlin() and, possibly, getColor()
float interpolate(float A, float B, float alpha) {
    float ease = (3*alpha*alpha) - (2*alpha*alpha*alpha);
    return A + ease*(B-A);
}

// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value, z, by sampling a noise function
float Terrain::getHeight(float x, float y) {
    float z = computePerlin(x * 512, y * 512) / 512;
    return z + getHeightModification(x, y);
}


// Computes the normal of a vertex by averaging neighbors
glm::vec3 Terrain::getNormal(int row, int col) {
    glm::vec3 V = getPosition(row, col);
    glm::vec3 normals = {0, 0, 0};
    std::vector<std::vector<int>> neighbors = {
        {-1, -1}, {0, -1}, {1, -1}, {1, 0},
        {1, 1}, {0, 1}, {-1, 1}, {-1, 0}
    };

    for (int i = 0; i < 8; ++i) {
        int niRowOffset = neighbors[i][0];
        int niColOffset = neighbors[i][1];
        int ni2RowOffset = neighbors[(i + 1) % 8][0];
        int ni2ColOffset = neighbors[(i + 1) % 8][1];
        glm::vec3 ni = getPosition(row + niRowOffset, col + niColOffset);
        glm::vec3 ni2 = getPosition(row + ni2RowOffset, col + ni2ColOffset);
        normals += glm::cross(ni - V, ni2 - V);
    }
    return glm::normalize(normals);
}

// Computes color of vertex using normal and, optionally, position
glm::vec3 Terrain::getColor(glm::vec3 normal, glm::vec3 position) {
    float a = glm::clamp((position.z + .5f) / (.5f + .5f), 0.0f, 1.0f);
    float ease = (3 * a * a) - (2 * a * a * a);
    glm::vec3 color = {.95, .9, .6};
    color = {interpolate(color.r, ease, ease), interpolate(color.g, ease, ease),
             interpolate(color.b, ease, ease)};

    a = glm::dot(normal, glm::vec3(0, 0, 1));
    ease = (3 * a * a) - (2 * a * a * a);
    return glm::vec3(interpolate(1.f, color.r, ease), interpolate(1.f, color.g, ease),
                     interpolate(1.f, color.b, ease));
}

// Computes the intensity of Perlin noise at some point
float Terrain::computePerlin(float x, float y) {
    glm::vec2 p1 = {std::floor(x), std::floor(y)};
    glm::vec2 p2 = {std::floor(x) + 1, std::floor(y)};
    glm::vec2 p3 = {std::floor(x), std::floor(y) + 1};
    glm::vec2 p4 = {std::floor(x) + 1, std::floor(y) + 1};

    glm::vec2 offset1 = {p1.x - x, p1.y - y};
    glm::vec2 offset2 = {p2.x - x, p2.y - y};
    glm::vec2 offset3 = {p3.x - x, p3.y - y};
    glm::vec2 offset4 = {p4.x - x, p4.y - y};

    float A = glm::dot(sampleRandomVector(p3.x, p3.y), offset3);
    float B = glm::dot(sampleRandomVector(p4.x, p4.y), offset4);
    float C = glm::dot(sampleRandomVector(p2.x, p2.y), offset2);
    float D = glm::dot(sampleRandomVector(p1.x, p1.y), offset1);

    return interpolate(interpolate(D, C, x - std::floor(x)), interpolate(A, B, x - std::floor(x)), y - std::floor(y));
}
