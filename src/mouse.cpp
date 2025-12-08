#include "mouse.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <limits>

mouse::mouse() {}

// Möller Trumbore paper: https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
bool rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                           const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                           float& t, glm::vec3& intersection) {
    const float EPSILON = 0.0000001f;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDirection, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDirection, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * glm::dot(edge2, q);

    if (t > EPSILON) {
        intersection = rayOrigin + rayDirection * t;
        return true;
    }

    return false;
}

// https://antongerdelan.net/opengl/raycasting.html
std::optional<glm::vec3> mouse::mouse_click_callback(int b, int s, int mouse_x, int mouse_y, float width, float height,
                                                     glm::mat4 proj, glm::mat4 view, const std::vector<float>& terrainVerts,
                                                     int resolution, const glm::mat4& worldMatrix) {

    float x = (2.0f * mouse_x) / width - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / height;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    ray_world = glm::normalize(ray_world);

    glm::vec3 cameraPos = glm::vec3(glm::inverse(view) * glm::vec4(0, 0, 0, 1));

    // look for intersection with plane
    float closestT = std::numeric_limits<float>::max();
    glm::vec3 closestIntersection;
    bool foundIntersection = false;

    int numTriangles = resolution * resolution * 2;
    for (int i = 0; i < numTriangles; i++) {
        int baseIdx = i * 27;

        glm::vec3 v0 = glm::vec3(terrainVerts[baseIdx], terrainVerts[baseIdx + 1], terrainVerts[baseIdx + 2]);
        glm::vec3 v1 = glm::vec3(terrainVerts[baseIdx + 9], terrainVerts[baseIdx + 10], terrainVerts[baseIdx + 11]);
        glm::vec3 v2 = glm::vec3(terrainVerts[baseIdx + 18], terrainVerts[baseIdx + 19], terrainVerts[baseIdx + 20]);

        v0 = glm::vec3(worldMatrix * glm::vec4(v0, 1.0f));
        v1 = glm::vec3(worldMatrix * glm::vec4(v1, 1.0f));
        v2 = glm::vec3(worldMatrix * glm::vec4(v2, 1.0f));

        float t;
        glm::vec3 intersection;
        if (rayIntersectsTriangle(cameraPos, ray_world, v0, v1, v2, t, intersection)) {
            if (t < closestT) {
                closestT = t;
                closestIntersection = intersection;
                foundIntersection = true;
            }
        }
    }

    if (foundIntersection) {
        //std::cout<<closestIntersection.x<<std::endl;
        return closestIntersection;
    } else {
        return std::nullopt;
    }
}
