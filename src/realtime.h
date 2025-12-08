#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <unordered_set>
#include "utils/sceneparser.h"
#include "utils/camera.h"
#include "utils/sphere.h"
#include "utils/cone.h"
#include "utils/cube.h"
#include "utils/cylinder.h"
#include "utils/shaderloader.h"
#include "terrain.h"
#include "skybox.h"


class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

    // ========== SARYA: TERRAIN OBJECT SYSTEM ==========
    enum class LSystem {
        // if you want to replace the PrimitiveType enum with an L System type enum
    };

    struct TerrainObject {
        PrimitiveType type;              // Type of object
        glm::vec2 terrainPosition;    // Position on terrain (0-1 space)
        glm::mat4 modelMatrix;        // Full transformation matrix
        float size;                   // Object scale
        glm::vec4 color;              // Object color
        GLuint vbo;                   // Vertex buffer object
        GLuint vao;                   // Vertex array object
        GLsizei vertexCount;          // Number of vertices
    };

    std::vector<TerrainObject> m_terrainObjects;

    // Place an object on the terrain at given coordinates
    // SARYA: probably where the most changes will be
    void placeObjectOnTerrain(float terrainX, float terrainY, PrimitiveType type, float size = 0.05f);

    // Clear all terrain objects
    void clearTerrainObjects();

public slots:
    void tick(QTimerEvent* event);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // Tick Related Variables
    int m_timer;
    QElapsedTimer m_elapsedTimer;

    // Input Related Variables
    bool m_mouseDown = false;
    glm::vec2 m_prev_mouse_pos;
    std::unordered_map<Qt::Key, bool> m_keyMap;

    // Device Correction Variables
    int m_devicePixelRatio;

    // Original Realtime Variables
    GLuint m_shader;

    // Shape VBOs and VAOs
    GLuint m_sphere_vbo;
    GLuint m_sphere_vao;
    GLuint m_cone_vbo;
    GLuint m_cone_vao;
    GLuint m_cube_vbo;
    GLuint m_cube_vao;
    GLuint m_cylinder_vbo;
    GLuint m_cylinder_vao;

    // Shape data
    std::vector<float> m_sphere_data;
    std::vector<float> m_cone_data;
    std::vector<float> m_cube_data;
    std::vector<float> m_cylinder_data;

    // Setup helpers
    void bindData(std::vector<float> &shapeData, GLuint &vbo);
    void setUpBindings(std::vector<float> &shapeData, GLuint &vbo, GLuint &vao);
    void makeShapes();
    void updateShapes();
    void setUp();
    bool isSetUp = false;

    // Type interpretation
    GLuint typeInterpretVao(PrimitiveType type);
    GLsizei typeInterpretVertices(PrimitiveType type);

    // Camera and scene
    Camera camera;
    RenderData renderData;
    glm::vec3 camLook;
    glm::vec3 camUp;
    glm::vec4 camPos;

    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::mat4 m_mvp;
    glm::mat4 m_model;
    glm::mat3 ictm;

    float m_ka;
    float m_kd;
    float m_ks;

    void updateCamera();
    void updateLights();

    // Shadow mapping variables
    GLuint m_depthMapFBO;
    GLuint m_depthMap;
    GLuint m_depthShader;
    glm::mat4 m_lightSpaceMatrix;

    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;

    // Cached uniform locations for performance
    struct UniformLocations {
        GLint model;
        GLint view;
        GLint projection;
        GLint lightSpaceMatrix;
        GLint shadowMap;
        GLint lightPos;
        GLint mvp;
        GLint ictm;
        GLint cameraPos;
        GLint ka;
        GLint kd;
        GLint ks;
        GLint cAmbient;
        GLint cDiffuse;
        GLint cSpecular;
        GLint shininess;
        GLint cReflective;
    } m_uniformLocs;

    struct DepthUniformLocations {
        GLint lightSpaceMatrix;
        GLint model;
    } m_depthUniformLocs;

    void cacheUniformLocations();

    // ========== TERRAIN VARIABLES ==========
    QOpenGLShaderProgram *m_terrainProgram = nullptr;
    QOpenGLVertexArrayObject m_terrainVao;
    QOpenGLBuffer m_terrainVbo;

    Terrain m_terrain;
    std::vector<GLfloat> m_terrainVerts;

    int m_terrainProjMatrixLoc;
    int m_terrainMvMatrixLoc;
    int m_terrainWireshadeLoc;

    QMatrix4x4 m_terrainWorld;
    QMatrix4x4 m_terrainCamera;
    QMatrix4x4 m_terrainProj;

    glm::mat4 m_terrainViewMatrix;
    glm::mat4 m_terrainProjMatrix;
    glm::mat4 m_terrainWorldMatrix;

    float m_angleX;
    float m_angleY;
    float m_zoom;
    int m_intersected;
    glm::vec3 m_hitPoint;
    QPoint m_prevMousePosQt;

    int m_w;
    int m_h;

    bool m_showTerrain;
    bool m_placeObjectMode = false;
    PrimitiveType m_currentObjectType = PrimitiveType::PRIMITIVE_CUBE;

    // Terrain methods
    void initializeTerrain();
    void rebuildTerrainMatrices();
    void updateAffectedTiles(const std::unordered_set<int>& affectedTiles);
    void updateAffectedTiles(float x, float y, float radius);

    // Helper methods for terrain object system
    std::vector<float> getVertexDataForType(PrimitiveType type);
    std::string getObjectTypeName(PrimitiveType type);


    // skybox
    skybox m_skybox;
};
