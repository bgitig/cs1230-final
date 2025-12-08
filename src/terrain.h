#ifndef Terrain_H
#define Terrain_H

#include <vector>
#include <map>
#include "glm/glm.hpp"

struct TerrainTile {
    std::vector<float> vertices;
    int tileX;
    int tileY;
    bool needsUpdate;
};

class Terrain
{
public:
    Terrain();
    ~Terrain();

    std::vector<float> generateTerrain();
    std::vector<float> generateTile(int tileX, int tileY);
    glm::vec3 getPosition(int row, int col);
    glm::vec3 getNormal(int row, int col);
    glm::vec3 getColor(glm::vec3 normal, glm::vec3 position);

    int getResolution() { return m_resolution; }
    int getTilesPerSide() { return m_tilesPerSide; }
    int getTileResolution() { return m_tileResolution; }

    // New methods for terrain deformation
    void divot(float x, float y, float depth, float radius);
    float getHeightModification(float x, float y);

    // Tile management
    void getTileCoordinates(float x, float y, int& tileX, int& tileY);
    std::vector<int> getAffectedTiles(float x, float y, float radius);
    void updateTile(int tileX, int tileY, std::vector<float>& allVerts);

    bool m_wireshade;

private:
    float getHeight(float x, float y);
    float computePerlin(float x, float y);
    glm::vec2 sampleRandomVector(int row, int col);

    std::vector<glm::vec2> m_randVecLookup;
    int m_resolution;        // Total resolution (e.g., 100)
    int m_tilesPerSide;      // Number of tiles per side (e.g., 10)
    int m_tileResolution;    // Resolution per tile (e.g., 10)
    int m_lookupSize;

    // Store crater/divot information: (x, y, depth, radius)
    std::vector<glm::vec4> m_displacements;
};

#endif // Terrain_H
