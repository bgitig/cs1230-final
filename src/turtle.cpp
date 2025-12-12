#include "turtle.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

turtle::turtle() {
    thickness = 0.03f;
    branch_contraction_ratio = 0.999;
    tapering_ratio = 0.97f;
    len = 0.1f;
    m_currentIteration = 1;
    m_bracketDepth = 0;
    reset();
}

void turtle::reset() {
    m_state.position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_state.H = glm::vec3(0.0f, 1.0f, 0.0f);
    m_state.L = glm::vec3(-1.0f, 0.0f, 0.0f);
    m_state.U = glm::vec3(0.0f, 0.0f, 1.0f);

    thickness = 0.03f;
    m_state.thickness = thickness;

    while (!states.empty()) states.pop();
    m_vertexData.clear();
    m_currentIteration = 1;
    m_bracketDepth = 0;
}

void turtle::setThickness(int val) {
    if (val == 0) {
        thickness *= branch_contraction_ratio;
        m_state.thickness = thickness;
    } else if (val == -1) {
        thickness *= tapering_ratio;
        m_state.thickness = thickness;
    } else {
        thickness = val;
        m_state.thickness = thickness;
    }
}

void turtle::draw() {
    glm::vec3 start = m_state.position;
    moveForward(len);
    glm::vec3 end = m_state.position;

    drawCylinder(start, end, m_state.thickness);
}

void turtle::moveForward(float dist) {
    m_state.position += m_state.H * dist;
}

void turtle::calculateRotationMatrix(float angle) {
    float rad = glm::radians(angle);
    float cosang = glm::cos(rad);
    float sinang = glm::sin(rad);

    m_R_U = glm::mat3(
        cosang,     sinang,     0.0f,
        -sinang,    cosang,     0.0f,
        0.0f,       0.0f,       1.0f
        );

    m_R_L = glm::mat3(
        cosang,  0.0f,  -sinang,
        0.0f,    1.0f,   0.0f,
        sinang,  0.0f,   cosang
        );

    m_R_H = glm::mat3(
        1.0f,  0.0f,     0.0f,
        0.0f,  cosang,  -sinang,
        0.0f,  sinang,   cosang
        );
}

void turtle::rotate(float angle) {
    calculateRotationMatrix(angle);
    m_state.H = m_R_U * m_state.H;
    m_state.L = m_R_U * m_state.L;

    m_state.H = glm::normalize(m_state.H);
    m_state.L = glm::normalize(m_state.L);
}

void turtle::pitchDown(float angle) {
    calculateRotationMatrix(angle);
    m_state.H = m_R_L * m_state.H;
    m_state.U = m_R_L * m_state.U;

    m_state.H = glm::normalize(m_state.H);
    m_state.U = glm::normalize(m_state.U);
}

void turtle::pitchUp(float angle) {
    pitchDown(-angle);
}

void turtle::rollLeft(float angle) {
    calculateRotationMatrix(angle);
    m_state.L = m_R_H * m_state.L;
    m_state.U = m_R_H * m_state.U;

    m_state.L = glm::normalize(m_state.L);
    m_state.U = glm::normalize(m_state.U);
}

void turtle::rollRight(float angle) {
    rollLeft(-angle);
}

void turtle::turnAround() {
    rotate(180.0f);
}

void turtle::saveState() {
    states.push(m_state);
}

void turtle::restoreState() {
    if (states.empty()) {
        std::cout << "No saved state available" << std::endl;
        return;
    }
    m_state = states.top();
    states.pop();
}

void turtle::drawCylinder(const glm::vec3& start, const glm::vec3& end, float radius) {
    glm::vec3 direction = end - start;
    float length = glm::length(direction);

    if (length < 0.0001f) return;

    glm::vec3 axis = glm::normalize(direction);

    glm::vec3 perp1, perp2;
    if (abs(axis.y) < 0.9f) {
        perp1 = glm::normalize(glm::cross(axis, glm::vec3(0, 1, 0)));
    } else {
        perp1 = glm::normalize(glm::cross(axis, glm::vec3(1, 0, 0)));
    }
    perp2 = glm::normalize(glm::cross(axis, perp1));

    int segments = 12;
    float angleStep = 2.0f * M_PI / segments;

    // ===== CYLINDER SIDES (existing code) =====
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;

        glm::vec3 offset1 = radius * ((float)cos(angle1) * perp1 + (float)sin(angle1) * perp2);
        glm::vec3 offset2 = radius * ((float)cos(angle2) * perp1 + (float)sin(angle2) * perp2);

        glm::vec3 p1 = start + offset1;
        glm::vec3 p2 = start + offset2;
        glm::vec3 p3 = end + offset1;
        glm::vec3 p4 = end + offset2;

        glm::vec3 normal1 = glm::normalize(offset1);
        glm::vec3 normal2 = glm::normalize(offset2);

        // Triangle 1
        m_vertexData.push_back(p1.x); m_vertexData.push_back(p1.y); m_vertexData.push_back(p1.z);
        m_vertexData.push_back(normal1.x); m_vertexData.push_back(normal1.y); m_vertexData.push_back(normal1.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p2.x); m_vertexData.push_back(p2.y); m_vertexData.push_back(p2.z);
        m_vertexData.push_back(normal2.x); m_vertexData.push_back(normal2.y); m_vertexData.push_back(normal2.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p3.x); m_vertexData.push_back(p3.y); m_vertexData.push_back(p3.z);
        m_vertexData.push_back(normal1.x); m_vertexData.push_back(normal1.y); m_vertexData.push_back(normal1.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        // Triangle 2
        m_vertexData.push_back(p2.x); m_vertexData.push_back(p2.y); m_vertexData.push_back(p2.z);
        m_vertexData.push_back(normal2.x); m_vertexData.push_back(normal2.y); m_vertexData.push_back(normal2.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p4.x); m_vertexData.push_back(p4.y); m_vertexData.push_back(p4.z);
        m_vertexData.push_back(normal2.x); m_vertexData.push_back(normal2.y); m_vertexData.push_back(normal2.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p3.x); m_vertexData.push_back(p3.y); m_vertexData.push_back(p3.z);
        m_vertexData.push_back(normal1.x); m_vertexData.push_back(normal1.y); m_vertexData.push_back(normal1.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));
    }

    // ===== ADD BOTTOM CAP (START) =====
    glm::vec3 bottomNormal = -axis;  // Points backward
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;

        glm::vec3 offset1 = (float)radius * ((float)cos(angle1) * perp1 + (float)sin(angle1) * perp2);
        glm::vec3 offset2 = (float)radius * ((float)cos(angle2) * perp1 + (float)sin(angle2) * perp2);

        glm::vec3 p1 = start + offset1;
        glm::vec3 p2 = start + offset2;

        // Triangle fan from center
        m_vertexData.push_back(start.x); m_vertexData.push_back(start.y); m_vertexData.push_back(start.z);
        m_vertexData.push_back(bottomNormal.x); m_vertexData.push_back(bottomNormal.y); m_vertexData.push_back(bottomNormal.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p2.x); m_vertexData.push_back(p2.y); m_vertexData.push_back(p2.z);
        m_vertexData.push_back(bottomNormal.x); m_vertexData.push_back(bottomNormal.y); m_vertexData.push_back(bottomNormal.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p1.x); m_vertexData.push_back(p1.y); m_vertexData.push_back(p1.z);
        m_vertexData.push_back(bottomNormal.x); m_vertexData.push_back(bottomNormal.y); m_vertexData.push_back(bottomNormal.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));
    }

    // ===== ADD TOP CAP (END) =====
    glm::vec3 topNormal = axis;  // Points forward
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;

        glm::vec3 offset1 = (float)radius * ((float)cos(angle1) * perp1 + (float)sin(angle1) * perp2);
        glm::vec3 offset2 = (float)radius * ((float)cos(angle2) * perp1 + (float)sin(angle2) * perp2);

        glm::vec3 p1 = end + offset1;
        glm::vec3 p2 = end + offset2;

        // Triangle fan from center
        m_vertexData.push_back(end.x); m_vertexData.push_back(end.y); m_vertexData.push_back(end.z);
        m_vertexData.push_back(topNormal.x); m_vertexData.push_back(topNormal.y); m_vertexData.push_back(topNormal.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p1.x); m_vertexData.push_back(p1.y); m_vertexData.push_back(p1.z);
        m_vertexData.push_back(topNormal.x); m_vertexData.push_back(topNormal.y); m_vertexData.push_back(topNormal.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));

        m_vertexData.push_back(p2.x); m_vertexData.push_back(p2.y); m_vertexData.push_back(p2.z);
        m_vertexData.push_back(topNormal.x); m_vertexData.push_back(topNormal.y); m_vertexData.push_back(topNormal.z);
        m_vertexData.push_back(static_cast<float>(m_currentIteration));
    }
}
