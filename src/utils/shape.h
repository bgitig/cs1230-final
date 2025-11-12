#ifndef SHAPE_H
#define SHAPE_H
#include <vector>
#include <glm/glm.hpp>

class Shape {
protected:
    int m_param1 = 1;
    int m_param2 = 1;
    std::vector<float> m_vertexData;

    void insertVec3(std::vector<float> &data, const glm::vec3 &v);

public:
    Shape(int param1, int param2) : m_param1(param1), m_param2(param2) {}
    virtual ~Shape() = default;

    virtual void setVertexData() = 0;
    const std::vector<float>& getVertexData() const {return m_vertexData;}


    virtual void makeTile(glm::vec3 topLeft,
                          glm::vec3 topRight,
                          glm::vec3 bottomLeft,
                          glm::vec3 bottomRight) {}

    void updateParams(int param1, int param2);
};
#endif
