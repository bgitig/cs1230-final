#ifndef CYLINDER_H
#define CYLINDER_H
#include "Shape.h"

class Cylinder : public Shape {
private:
    glm::vec3 computeNormal(glm::vec3& p);

    void makeWedge(float currTheta, float nextTheta);
    void makeSide();
    void makeCap(float y, bool top);
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight) override;

public:
    Cylinder(int param1 = 1, int param2 = 3);
    void setVertexData() override;
};

#endif
