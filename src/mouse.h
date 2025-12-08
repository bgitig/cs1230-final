#ifndef MOUSE_H
#define MOUSE_H
#include "glm/glm.hpp"
#include <iostream>

class mouse
{
public:
    mouse();
    static std::optional<glm::vec3> mouse_click_callback(int b, int s, int mouse_x, int mouse_y,
                                                         float width, float height,
                                                         glm::mat4 proj, glm::mat4 view,
                                                         const std::vector<float>& terrainVerts,
                                                         int resolution, const glm::mat4& worldMatrix);};

#endif // MOUSE_H
