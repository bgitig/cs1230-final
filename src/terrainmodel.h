#ifndef TERRAINMODEL_H
#define TERRAINMODEL_H

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "objloader.h"
#include "terrain.h"

class TerrainModel {
public:
    TerrainModel();
    ~TerrainModel();

    // Load an OBJ model file
    bool loadFromFile(const std::string& filepath);

    // Place the model on the terrain at the given position
    void placeOnTerrain(Terrain* terrain, float terrainX, float terrainY, float scale = 1.0f);

    // Update position if terrain changes
    void updatePosition(Terrain* terrain);

    // Render the model
    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection,
                const glm::mat4& terrainWorldMatrix);

    // Cleanup OpenGL resources
    void cleanup();

    // Setters
    void setColor(const glm::vec4& color) { m_color = color; }
    void setScale(float scale);
    void setRotation(float angleRadians, const glm::vec3& axis);

    // Getters
    glm::vec2 getTerrainPosition() const { return m_terrainPos; }
    glm::vec3 getWorldPosition() const { return m_worldPos; }
    float getScale() const { return m_scale; }
    bool isLoaded() const { return m_isLoaded; }

private:
    void updateModelMatrix(Terrain* terrain);
    void setupOpenGL();

    OBJModel m_model;
    GLuint m_vao;
    GLuint m_vbo;

    glm::vec2 m_terrainPos;  // Position on terrain (0-1 range)
    glm::vec3 m_worldPos;    // Actual 3D world position
    glm::mat4 m_modelMatrix;

    float m_scale;
    glm::mat4 m_rotationMatrix;
    glm::vec4 m_color;

    bool m_isLoaded;
    bool m_isSetup;

    // Store terrain reference for updates
    Terrain* m_terrain;
};

#endif // TERRAINMODEL_H
