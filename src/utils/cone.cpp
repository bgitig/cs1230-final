#include "cone.h"

Cone::Cone(int param1, int param2)
    : Shape(param1, param2) {
    setVertexData();
}

void Cone::makeCapTile(glm::vec3 topLeft,
                       glm::vec3 topRight,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight) {
    glm::vec3 n(0.f, -1.f, 0.f);

    insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n);
    insertVec3(m_vertexData, bottomLeft); insertVec3(m_vertexData, n);
    insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n);

    insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n);
    insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n);
    insertVec3(m_vertexData, topRight);   insertVec3(m_vertexData, n);
}

void Cone::makeCapSlice(float currentTheta, float nextTheta) {
    float y = -0.5f;
    float radiusStep = 0.5f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float r1 = i * radiusStep;
        float r2 = (i + 1) * radiusStep;

        glm::vec3 topLeft( r1 * cos(currentTheta), y, r1 * sin(currentTheta) );
        glm::vec3 topRight( r1 * cos(nextTheta),   y, r1 * sin(nextTheta) );
        glm::vec3 bottomLeft( r2 * cos(currentTheta), y, r2 * sin(currentTheta) );
        glm::vec3 bottomRight( r2 * cos(nextTheta),  y, r2 * sin(nextTheta) );

        makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

glm::vec3 Cone::calcNorm(glm::vec3& pt) {
    if (sqrt(pt.x*pt.x+pt.z*pt.z) < 1e-4f) {
        return glm::normalize(glm::vec3(0.f, 0.5f, 0.f));
    }
    float xNorm = (2 * pt.x);
    float yNorm = -(1.f/4.f) * (2.f * pt.y - 1.f);
    float zNorm = (2 * pt.z);

    return glm::normalize(glm::vec3{ xNorm, yNorm, zNorm });
}

void Cone::makeSlopeTile(glm::vec3 topLeft,
                         glm::vec3 topRight,
                         glm::vec3 bottomLeft,
                         glm::vec3 bottomRight) {
    glm::vec3 n1 = calcNorm(topLeft);
    glm::vec3 n2 = calcNorm(bottomLeft);
    glm::vec3 n3 = calcNorm(bottomRight);
    glm::vec3 n4 = calcNorm(topRight);

    insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n3);
    insertVec3(m_vertexData, bottomLeft); insertVec3(m_vertexData, n2);

    insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, topRight);   insertVec3(m_vertexData, n4);
    insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n3);
}

void Cone::makeSlopeSlice(float currentTheta, float nextTheta) {
    for (int i = 0; i < m_param1; i++) {
        float tTop = (float)i / (float)m_param1;         // 0..(1 - 1/m_param1)
        float tBottom = (float)(i + 1) / (float)m_param1; // (1/m_param1)..1

        float yTop = 0.5f - tTop * 1.0f;     // 0.5 -> -0.5
        float yBottom = 0.5f - tBottom * 1.0f;

        float rTop = 0.5f * tTop;            // 0 -> 0.5
        float rBottom = 0.5f * tBottom;

        glm::vec3 topLeft(  rTop * cos(currentTheta),  yTop,  rTop * sin(currentTheta) );
        glm::vec3 topRight( rTop * cos(nextTheta),     yTop,  rTop * sin(nextTheta) );
        glm::vec3 bottomLeft( rBottom * cos(currentTheta), yBottom, rBottom * sin(currentTheta) );
        glm::vec3 bottomRight(rBottom * cos(nextTheta),  yBottom, rBottom * sin(nextTheta) );

        makeSlopeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    makeCapSlice(currentTheta, nextTheta);
    makeSlopeSlice(currentTheta, nextTheta);
}


void Cone::setVertexData() {
    // TODO for Project 5: Lights, Camera
    int p2 = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / p2);
    for (int i = 0; i < p2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}
