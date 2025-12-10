#ifndef Particles_H
#define Particles_H

#include <GL/glew.h>
#include <QOpenGLWidget>
#include <QTimer>
#include <QTime>
#include <glm/glm.hpp>
#include <vector>

// CPU representation of a particle
struct ParticleBurst {
    glm::vec3 pos, speed;
    glm::vec4 color;
    float size, angle, weight;
    float life;
    float cameradistance;

    ParticleBurst(): pos(0.0f), speed(0.0f), color(1.0f), life(0.0f) {}

    bool operator<(const ParticleBurst& that) const {
        return this->cameradistance > that.cameradistance;
    }
};

class Particles : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit Particles(QWidget *parent = nullptr);
    ~Particles();
    void triggerBurst(const glm::vec3& position);
    void setTerrainMatrices(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& world);
    void initializeGL() override;
    void paintGL() override;

protected:

    void resizeGL(int w, int h) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void updateScene();

private:
    bool loadShaders();
    int findUnusedParticle();
    void sortParticles();
    void updateParticles(double delta);

    GLuint m_programID;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;

    GLint m_viewProjMatrixID;
    GLint m_cameraRightWorldspaceID;
    GLint m_cameraUpWorldspaceID;

    GLuint m_billboardVertexBuffer;
    GLuint m_particlesPositionBuffer;
    GLuint m_particlesColorBuffer;
    GLuint m_vao;

    static const int MaxParticles = 10000;
    ParticleBurst m_particlesContainer[MaxParticles];
    int m_lastUsedParticle;
    int m_particlesCount;

    GLfloat* m_particlePositionSizeData;
    GLubyte* m_particleColorData;

    glm::vec3 m_cameraPosition;
    float m_horizontalAngle;
    float m_verticalAngle;
    float m_radius;

    // still working on time
    QTimer m_timer;

    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;


    glm::mat4 m_externalViewMatrix;
    glm::mat4 m_externalProjMatrix;
    glm::mat4 m_externalWorldMatrix;
    bool m_useExternalMatrices;
};

#endif // Particles_H
