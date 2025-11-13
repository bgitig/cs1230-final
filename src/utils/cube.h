#ifndef CUBE_H
#define CUBE_H

#include "Shape.h"

class Cube : public Shape {
private:
    void makeFace(glm::vec3 topLeft,
              glm::vec3 topRight,
              glm::vec3 bottomLeft,
              glm::vec3 bottomRigh);
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight) override;

public:
    Cube(int param1 = 1, int param2 = 1);
    void setVertexData() override;
};

#endif
