#include "cube.h"

Cube::Cube(int param1, int param2) : Shape (param1, param2) {
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.
    glm::vec3 v1(topLeft); glm::vec3 v2(bottomLeft); glm::vec3 v3(bottomRight);
    glm::vec3 n1(glm::normalize(glm::cross(v1-v2, v1-v3)));
    glm::vec3 n2(glm::normalize(glm::cross(v2-v3, v2-v1)));
    glm::vec3 n3(glm::normalize(glm::cross(v3-v1, v3-v2)));

    insertVec3(m_vertexData, v1);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, v2);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, v3);
    insertVec3(m_vertexData, n3);

    v1 = topLeft; v2 = bottomRight; v3 = topRight;
    n1 = glm::normalize(glm::cross(v1-v2, v1-v3));
    n2 = glm::normalize(glm::cross(v2-v3, v2-v1));
    n3 = glm::normalize(glm::cross(v3-v1, v3-v2));

    insertVec3(m_vertexData, v1);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, v2);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, v3);
    insertVec3(m_vertexData, n3);

}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 3: create a single side of the cube out of the 4
    //         given points and makeTile()
    // Note: think about how param 1 affects the number of triangles on
    //       the face of the cube
    float step = 1.0f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {
            float u0 = j * step;
            float u1 = (j + 1) * step;
            float v0 = i * step;
            float v1 = (i + 1) * step;

            glm::vec3 tl = glm::mix(
                glm::mix(topLeft, topRight, u0),
                glm::mix(bottomLeft, bottomRight, u0),
                v0
                );

            glm::vec3 tr = glm::mix(
                glm::mix(topLeft, topRight, u1),
                glm::mix(bottomLeft, bottomRight, u1),
                v0
                );

            glm::vec3 bl = glm::mix(
                glm::mix(topLeft, topRight, u0),
                glm::mix(bottomLeft, bottomRight, u0),
                v1
                );

            glm::vec3 br = glm::mix(
                glm::mix(topLeft, topRight, u1),
                glm::mix(bottomLeft, bottomRight, u1),
                v1
                );

            makeTile(tl, tr, bl, br);
        }
    }
}

void Cube::setVertexData() {
    // Uncomment these lines for Task 2, then comment them out for Task 3:

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));

    // Uncomment these lines for Task 3:

    // makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));

    // Task 4: Use the makeFace() function to make all 6 sides of the cube

    //z
    makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),   // top left
             glm::vec3( 0.5f,  0.5f,  0.5f),   // top right
             glm::vec3(-0.5f, -0.5f,  0.5f),   // bottom left
             glm::vec3( 0.5f, -0.5f,  0.5f));  // bottom right

    // -z
    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    // -x
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f));

    // x
    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));

    // y
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f));

    // -y
    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));

}
