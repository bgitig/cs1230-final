#include "cylinder.h"

Cylinder::Cylinder(int param1, int param2)
    : Shape(param1, param2) {
    setVertexData();
}

glm::vec3 Cylinder::computeNormal(glm::vec3& p) {
    return glm::normalize(glm::vec3(p.x, 0.f, p.z));
}

void Cylinder::makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {
    glm::vec3 n1 = computeNormal(topLeft);
    glm::vec3 n2 = computeNormal(bottomLeft);
    glm::vec3 n3 = computeNormal(bottomRight);
    glm::vec3 n4 = computeNormal(topRight);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, n3);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, n3);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, n4);
}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    float yStep = 1.f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float yTop = 0.5f - i * yStep;
        float yBottom = 0.5f - (i + 1) * yStep;

        glm::vec3 topLeft(0.5f * glm::cos(currentTheta), yTop, -0.5f * glm::sin(currentTheta));
        glm::vec3 topRight(0.5f * glm::cos(nextTheta), yTop, -0.5f * glm::sin(nextTheta));
        glm::vec3 bottomLeft(0.5f * glm::cos(currentTheta), yBottom, -0.5f * glm::sin(currentTheta));
        glm::vec3 bottomRight(0.5f * glm::cos(nextTheta), yBottom, -0.5f * glm::sin(nextTheta));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cylinder::makeSide() {
    int p2 = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / p2);
    for (int i = 0; i < p2; i++) {
        makeWedge(thetaStep * i, thetaStep * (i + 1));
    }
}

void Cylinder::makeCap(float y, bool top) {
    int p2 = std::max(3, m_param2);

    float thetaStep = glm::radians(360.f / p2);
    float radiusStep = 0.5f / m_param1;

    glm::vec3 center(0.f, y, 0.f);
    glm::vec3 n = top ? glm::vec3(0.f, 1.0f, 0.f) : glm::vec3(0.f, -1.0f, 0.f);

    for (int i = 0; i < p2; i++) {
        float t1 = thetaStep * i;
        float t2 = thetaStep * (i + 1);

        for (int j = 0; j < m_param1; j++) {
            float r1 = j * radiusStep;
            float r2 = (j + 1) * radiusStep;

            glm::vec3 topLeft(  r1 * glm::cos(t1), y, -r1 * glm::sin(t1) );
            glm::vec3 topRight( r1 * glm::cos(t2), y, -r1 * glm::sin(t2) );
            glm::vec3 bottomLeft(  r2 * glm::cos(t1), y, -r2 * glm::sin(t1) );
            glm::vec3 bottomRight( r2 * glm::cos(t2), y, -r2 * glm::sin(t2) );

            if (top) {
                insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, bottomLeft); insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n);

                insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, topRight);   insertVec3(m_vertexData, n);
            } else {
                insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, bottomLeft); insertVec3(m_vertexData, n);

                insertVec3(m_vertexData, topLeft);    insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, topRight);   insertVec3(m_vertexData, n);
                insertVec3(m_vertexData, bottomRight);insertVec3(m_vertexData, n);
            }
        }
    }
}

void Cylinder::setVertexData() {
    // TODO for Project 5: Lights, Camera
    makeSide();
    makeCap(0.5f, true);
    makeCap(-0.5f, false);
}
