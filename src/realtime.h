#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "utils/camera.h"
#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "utils/shaderloader.h"
#include "utils/shape.h"
#include "utils/sphere.h"
// #include "utils/cube.h"
// #include "utils/cone.h"
// #include "utils/cylinder.h"

struct Mesh {
    GLuint vbo;
    GLuint vao;
    int vertexCount;
    RenderShapeData shape;
};

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);


public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    Mesh createMesh(const std::vector<GLfloat> &shapeData);


    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;

    RenderData renderData;
    Camera camera;
    glm::vec3 camLook = glm::vec3(1.0f);
    glm::vec3 camUp = glm::vec3(1.0f);
    glm::vec4 camPos = glm::vec4(1.0f);

    GLuint m_shader;
    std::vector<Mesh> m_meshes;

    // from lab 10
    glm::mat4 m_model = glm::mat4(1.0f);
    glm::mat4 m_view  = glm::mat4(1.0f);
    glm::mat4 m_proj  = glm::mat4(1.0f);
    glm::mat4 m_mvp = glm::mat4(1.0f);

    GLuint vbo;
    GLuint vao;

    glm::vec4 m_lightPos;

    float m_ka;
    float m_kd;
    float m_ks;
    float m_shininess;

    QPoint m_prevMousePos;
    float  m_angleX;
    float  m_angleY;
    float  m_zoom;
};
