#include "shape.h"

void Shape::updateParams(int param1, int param2) {
    m_param1 = param1;
    m_param2 = param2;
    m_vertexData.clear();
    setVertexData();
}

void Shape::insertVec3(std::vector<float> &data, const glm::vec3 &v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
