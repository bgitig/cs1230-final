#ifndef SPHERE_H
#define SPHERE_H
#include "Shape.h"

class Sphere : public Shape {
private:
    void makeWedge(float currTheta, float nextTheta);
    void makeSphere();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight) override;

public:
    Sphere(int param1 = 2, int param2 = 3);
    void setVertexData() override;
};

#endif
