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
#include "utils/cube.h"
#include "utils/cone.h"
#include "utils/cylinder.h"

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

    void bindData(std::vector<float> &shapeData, GLuint &vbo);
    void setUpBindings(std::vector<float> &shapeData, GLuint &vbo, GLuint &vao);
    void makeShapes();
    void updateShapes();
    void makeFBOs();
    void setUp();
    GLuint typeInterpretVao(PrimitiveType type);
    GLsizei typeInterpretVertices(PrimitiveType type);
    void updateCamera();
    void updateLights();

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;

    GLuint m_shader;

    // camera data
    RenderData renderData;
    Camera camera;
    glm::vec4 camera_pos;
    glm::vec3 camLook = glm::vec3(1.0f);
    glm::vec3 camUp = glm::vec3(1.0f);
    glm::vec4 camPos = glm::vec4(1.0f);


    // from lab 10
    glm::mat4 m_model = glm::mat4(1.0f);
    glm::mat4 m_view  = glm::mat4(1.0f);
    glm::mat4 m_proj  = glm::mat4(1.0f);
    glm::mat4 m_mvp = glm::mat4(1.0f);
    glm::mat3 ictm = glm::mat3(1.0f);

    // shape data
    bool isSetUp = false;
    GLuint m_sphere_vbo;
    GLuint m_sphere_vao;
    std::vector<float> m_sphere_data;
    GLuint m_cone_vbo;
    GLuint m_cone_vao;
    std::vector<float> m_cone_data;
    GLuint m_cube_vbo;
    GLuint m_cube_vao;
    std::vector<float> m_cube_data;
    GLuint m_cylinder_vbo;
    GLuint m_cylinder_vao;
    std::vector<float> m_cylinder_data;

    // lighting data
    glm::vec4 m_lightPos;

    float m_ka;
    float m_kd;
    float m_ks;

    QPoint m_prevMousePos;
    float  m_angleX;
    float  m_angleY;
    float  m_zoom;

    GLuint m_defaultFBO;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;

    GLsizei occW, occH;
    GLuint sceneFBO, sceneColorTex;
    GLuint occFBO, occTex;
    GLuint godraysFBO, godraysTex;
    GLuint quadVAO, quadVBO;
    GLuint occShader, godrayShader, compositeShader;

};
