#pragma once
#include <stack>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 3D Turtle state for L-system interpretation
struct TurtleState3D {
    glm::vec3 position;
    glm::vec3 H;  // Heading (forward direction)
    glm::vec3 L;  // Left direction
    glm::vec3 U;  // Up direction
    float thickness;
};

class turtle {
public:
    turtle();

    // 3D turtle operations
    void draw();                    // Draw forward with current thickness
    void moveForward(float dist);   // Move without drawing
    void rotate(float angle);       // Turn left (+) or right (-)
    void pitchDown(float angle);    // & operation
    void pitchUp(float angle);      // ^ operation
    void rollLeft(float angle);     // < operation
    void rollRight(float angle);    // > operation
    void turnAround();              // | operation

    void saveState();               // [ - push state
    void restoreState();            // ] - pop state

    // Parameters
    void setThickness(int val);
    void setStepLength(float length) { len = length; }
    void setBranchContractionRatio(float ratio) { branch_contraction_ratio = ratio; }
    void setTaperingRatio(float ratio) { tapering_ratio = ratio; }

    // Get generated geometry
    const std::vector<float>& getVertexData() const { return m_vertexData; }
    int getVertexCount() const { return m_vertexData.size() / 6; }

    // Reset for new drawing
    void reset();

private:
    // Current state
    TurtleState3D m_state;
    std::stack<TurtleState3D> states;

    // Parameters
    float thickness;
    float branch_contraction_ratio;
    float tapering_ratio;
    float len;  // step length

    // Geometry data (interleaved: pos.x, pos.y, pos.z, norm.x, norm.y, norm.z)
    std::vector<float> m_vertexData;

    // Rotation matrices (from reference document)
    glm::mat3 m_R_U;  // Rotation around U (yaw)
    glm::mat3 m_R_L;  // Rotation around L (pitch)
    glm::mat3 m_R_H;  // Rotation around H (roll)

    void calculateRotationMatrix(float angle);
    void drawCylinder(const glm::vec3& start, const glm::vec3& end, float radius);
};
