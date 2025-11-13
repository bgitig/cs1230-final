#ifndef CONE_H
#define CONE_H
#include "Shape.h"

class Cone : public Shape {
private:
    void makeCapTile(glm::vec3 topLeft,
                     glm::vec3 topRight,
                     glm::vec3 bottomLeft,
                     glm::vec3 bottomRight);
    void makeCapSlice(float currentTheta, float nextTheta);
    glm::vec3 calcNorm(glm::vec3& pt);
    void makeSlopeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                       glm::vec3 bottomRight);
    void makeSlopeSlice(float currentTheta, float nextTheta);
    void makeWedge(float currentTheta, float nextTheta);
        // void makeTile(glm::vec3 topLeft,
        //           glm::vec3 topRight,
        //           glm::vec3 bottomLeft,
        //           glm::vec3 bottomRight) override;

public:
    Cone(int param1 = 1, int param2 = 3);
    void setVertexData() override;
};

#endif
