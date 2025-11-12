#include "sphere.h"
#include <iostream>


Sphere::Sphere(int param1, int param2)
    : Shape(param1, param2) {
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!
    glm::vec3 v1(topLeft); glm::vec3 v2(bottomLeft); glm::vec3 v3(bottomRight);
    glm::vec3 n1(glm::normalize(v1));
    glm::vec3 n2(glm::normalize(v2));
    glm::vec3 n3(glm::normalize(v3));

    insertVec3(m_vertexData, v1);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, v2);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, v3);
    insertVec3(m_vertexData, n3);

    v1 = topLeft; v2 = bottomRight; v3 = topRight;
    n1 = glm::normalize(v1);
    n2 = glm::normalize(v2);
    n3 = glm::normalize(v3);

    insertVec3(m_vertexData, v1);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, v2);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, v3);
    insertVec3(m_vertexData, n3);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!
    std::cout<<"make wedge1"<<std::endl;

    int p1 = std::max(1, m_param1);

    float phiStep = glm::radians(180.f / p1);
    for (int i = 0; i< p1; i++) {
        float currentPhi = i * phiStep;
        float nextPhi = (i + 1) * phiStep;

        glm::vec3 tl(0.5f * glm::sin(currentPhi) * glm::cos(currentTheta),
                     0.5f * glm::cos(currentPhi),
                     -0.5f * glm::sin(currentPhi) * glm::sin(currentTheta));
        glm::vec3 tr(0.5f * glm::sin(currentPhi) * glm::cos(nextTheta),
                     0.5f * glm::cos(currentPhi),
                     -0.5f * glm::sin(currentPhi) * glm::sin(nextTheta));
        glm::vec3 bl(0.5f * glm::sin(nextPhi) * glm::cos(currentTheta),
                     0.5f * glm::cos(nextPhi),
                     -0.5f * glm::sin(nextPhi) * glm::sin(currentTheta));
        glm::vec3 br(0.5f * glm::sin(nextPhi) * glm::cos(nextTheta),
                     0.5f * glm::cos(nextPhi),
                     -0.5f * glm::sin(nextPhi) * glm::sin(nextTheta));

        makeTile(tl, tr, bl, br);
    }
    std::cout<<"make wedge2"<<std::endl;

}

void Sphere::makeSphere() {
    std::cout<<"makespehre1"<<std::endl;

    int p2 = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / p2);
    for (int i = 0; i < p2; i++) {
        makeWedge(thetaStep*i, thetaStep*(i+1));
    }
    std::cout<<"makespehre2"<<std::endl;

}

void Sphere::setVertexData() {
    std::cout<<"setting v data1"<<std::endl;
    makeSphere();
    std::cout<<"setting v data2"<<std::endl;

}
