#ifndef BUMPMAPPING_H
#define BUMPMAPPING_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class BumpMapping {
public:
    BumpMapping();
    ~BumpMapping();

    void initialize();
    void cleanup();

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    float getBumpStrength() const { return m_bumpStrength; }
    void setBumpStrength(float strength) { m_bumpStrength = strength; }

    float getBumpScale() const { return m_bumpScale; }
    void setBumpScale(float scale) { m_bumpScale = scale; }

    float getBumpVariation() const { return m_bumpVariation; }
    void setBumpVariation(float variation) { m_bumpVariation = variation; }

    float getBumpSeed() const { return m_bumpSeed; }
    void setBumpSeed(float seed) { m_bumpSeed = seed; }

    GLuint getShader() const { return m_bumpShader; }

    void setLightDirection(const glm::vec3& dir) { m_lightDir = dir; }
    glm::vec3 getLightDirection() const { return m_lightDir; }

    bool isInitialized() {return m_initialized;}

private:
    bool m_enabled;
    float m_bumpStrength;
    float m_bumpScale;        // Controls size of features
    float m_bumpVariation;    // Controls irregularity/randomness
    float m_bumpSeed;         // Offset for different variations
    glm::vec3 m_lightDir;
    GLuint m_bumpShader;
    bool m_initialized;
};

#endif
