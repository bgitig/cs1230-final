#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "glm/gtc/matrix_transform.hpp"
#include "mouse.h"
#include "terrain.h"


// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // Terrain initialization
    m_angleX = 0;
    m_angleY = 0;
    m_zoom = 1.0;
    m_intersected = 0;
    m_showTerrain = true;
    m_placeObjectMode = false;
    //m_currentModelIndex = -1;

    // If you must use this function, do not edit anything above this
}

void Realtime::bindData(std::vector<float> &shapeData, GLuint &vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, shapeData.size() * sizeof(GLfloat), shapeData.data(), GL_STATIC_DRAW);
}

void Realtime::setUpBindings(std::vector<float> &shapeData, GLuint &vbo, GLuint &vao) {
    glGenBuffers(1, &vbo);
    bindData(shapeData, vbo);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, reinterpret_cast<void *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, reinterpret_cast<void *>(sizeof(GLfloat)*3));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::initializeBaseModel() {
    // Load your OBJ file (adjust path as needed)
    std::string modelPath = ":/images/Base.obj";

    if (!OBJLoader::loadOBJ(modelPath, m_baseModel_data)) {
        std::cerr << "Failed to load base model!" << std::endl;
        return;
    }

    // Create VBO and VAO
    glGenBuffers(1, &m_baseModel_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_baseModel_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_baseModel_data.size() * sizeof(GLfloat),
                 m_baseModel_data.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_baseModel_vao);
    glBindVertexArray(m_baseModel_vao);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                          reinterpret_cast<void*>(0));

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                          reinterpret_cast<void*>(sizeof(GLfloat) * 3));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Position model under terrain
    m_baseModelMatrix = glm::mat4(1.0f);
    m_baseModelMatrix = glm::translate(m_baseModelMatrix, glm::vec3(0.5f, 0.575f, -0.1f)); // Adjust as needed
    m_baseModelMatrix = glm::scale(m_baseModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f)); // Adjust scale

    std::cout << "Base model initialized successfully" << std::endl;
}

void Realtime::triggerParticleBurst(float worldX, float worldY, float worldZ) {
    if (!m_particles) {
        return;
    }

    // Convert world coordinates to particle system coordinates
    glm::vec3 burstPosition(worldX, worldY, worldZ + 0.05f); // Slightly above surface

    // Trigger burst at this location
    m_particles->triggerBurst(burstPosition);
}

void Realtime::makeShapes() {
    m_sphere_data = Sphere(settings.shapeParameter1,settings.shapeParameter2).getVertexData();
    m_cone_data = Cone(settings.shapeParameter1,settings.shapeParameter2).getVertexData();
    m_cube_data = Cube(settings.shapeParameter1,settings.shapeParameter2).getVertexData();
    m_cylinder_data = Cylinder(settings.shapeParameter1,settings.shapeParameter2).getVertexData();
}

void Realtime::updateShapes() {
    makeShapes();
    bindData(m_sphere_data, m_sphere_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    bindData(m_cone_data, m_cone_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    bindData(m_cube_data, m_cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    bindData(m_cylinder_data, m_cylinder_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::setUp() {
    makeShapes();
    setUpBindings(m_sphere_data, m_sphere_vbo, m_sphere_vao);
    setUpBindings(m_cone_data, m_cone_vbo, m_cone_vao);
    setUpBindings(m_cube_data, m_cube_vbo, m_cube_vao);
    setUpBindings(m_cylinder_data, m_cylinder_vbo, m_cylinder_vao);
    isSetUp = true;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);
    glDeleteBuffers(1, &m_cone_vbo);
    glDeleteVertexArrays(1, &m_cone_vao);
    glDeleteBuffers(1, &m_cube_vbo);
    glDeleteVertexArrays(1, &m_cube_vao);
    glDeleteBuffers(1, &m_cylinder_vbo);
    glDeleteVertexArrays(1, &m_cylinder_vao);

    glDeleteProgram(m_shader);

    // Shadow cleanup
    glDeleteFramebuffers(1, &m_depthMapFBO);
    glDeleteTextures(1, &m_depthMap);
    glDeleteProgram(m_depthShader);

    // Terrain cleanup
    if (m_terrainVao.isCreated()) {
        m_terrainVao.destroy();
    }
    if (m_terrainVbo.isCreated()) {
        m_terrainVbo.destroy();
    }
    if (m_terrainProgram) {
        delete m_terrainProgram;
        m_terrainProgram = nullptr;
    }

    // Terrain objects cleanup
    for (const TerrainObject& obj : m_terrainObjects) {
        if (obj.isFlag && obj.flagSimulation) {
            obj.flagSimulation->cleanup();
            delete obj.flagSimulation;
        } else {
            glDeleteBuffers(1, &obj.vbo);
            glDeleteVertexArrays(1, &obj.vao);
        }
    }

    if (m_particles) {
        delete m_particles;
        m_particles = nullptr;
    }

    glDeleteTextures(1, &m_preprocessTexture);
    glDeleteRenderbuffers(1, &m_preprocessDepthRBO);
    glDeleteFramebuffers(1, &m_preprocessFBO);

    m_terrainObjects.clear();
    m_bumpMapping.cleanup();
    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    glClearColor(0,0,0,1);
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    setUp();


    // skybox!
    m_skybox.init();


    // Shadow mapping initialization
    glGenFramebuffers(1, &m_depthMapFBO);

    glGenTextures(1, &m_depthMap);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Shadow framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_depthShader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/shadows.vert",
        ":/resources/shaders/shadows.frag"
        );

    // Initialize preprocessing FBO
    glGenFramebuffers(1, &m_preprocessFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_preprocessFBO);

    // Create color texture
    glGenTextures(1, &m_preprocessTexture);
    glBindTexture(GL_TEXTURE_2D, m_preprocessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 size().width() * m_devicePixelRatio,
                 size().height() * m_devicePixelRatio,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_preprocessTexture, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_preprocessDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_preprocessDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          size().width() * m_devicePixelRatio,
                          size().height() * m_devicePixelRatio);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_preprocessDepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Preprocessing framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initializeTerrain();

    cacheUniformLocations();

    //TreeManager codes
    m_treeManager.initialize();

    //RockGenerator
    m_rockGenerator.initialize();
    m_bumpMapping.initialize();

    m_particles = nullptr;
    m_showParticles = false;

        initializeBaseModel();
}

void Realtime::cacheUniformLocations() {
    glUseProgram(m_shader);
    m_uniformLocs.model = glGetUniformLocation(m_shader, "model");
    m_uniformLocs.view = glGetUniformLocation(m_shader, "view");
    m_uniformLocs.projection = glGetUniformLocation(m_shader, "projection");
    m_uniformLocs.lightSpaceMatrix = glGetUniformLocation(m_shader, "lightSpaceMatrix");
    m_uniformLocs.shadowMap = glGetUniformLocation(m_shader, "shadowMap");
    m_uniformLocs.lightPos = glGetUniformLocation(m_shader, "lightPos");
    m_uniformLocs.mvp = glGetUniformLocation(m_shader, "m_mvp");
    m_uniformLocs.ictm = glGetUniformLocation(m_shader, "ictm");
    m_uniformLocs.cameraPos = glGetUniformLocation(m_shader, "camera_pos");
    m_uniformLocs.ka = glGetUniformLocation(m_shader, "m_ka");
    m_uniformLocs.kd = glGetUniformLocation(m_shader, "m_kd");
    m_uniformLocs.ks = glGetUniformLocation(m_shader, "m_ks");
    m_uniformLocs.cAmbient = glGetUniformLocation(m_shader, "m_cAmbient");
    m_uniformLocs.cDiffuse = glGetUniformLocation(m_shader, "m_cDiffuse");
    m_uniformLocs.cSpecular = glGetUniformLocation(m_shader, "m_cSpecular");
    m_uniformLocs.shininess = glGetUniformLocation(m_shader, "m_shininess");
    m_uniformLocs.cReflective = glGetUniformLocation(m_shader, "m_cReflective");
    glUseProgram(0);

    glUseProgram(m_depthShader);
    m_depthUniformLocs.lightSpaceMatrix = glGetUniformLocation(m_depthShader, "lightSpaceMatrix");
    m_depthUniformLocs.model = glGetUniformLocation(m_depthShader, "model");
    glUseProgram(0);
}

void Realtime::initializeTerrain() {
    m_terrainProgram = new QOpenGLShaderProgram;
    m_terrainProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,":/resources/shaders/terrain.vert");
    m_terrainProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,":/resources/shaders/terrain.frag");
    m_terrainProgram->link();
    m_terrainProgram->bind();

    m_terrainProjMatrixLoc = m_terrainProgram->uniformLocation("projMatrix");
    m_terrainMvMatrixLoc = m_terrainProgram->uniformLocation("mvMatrix");
    m_terrainWireshadeLoc = m_terrainProgram->uniformLocation("wireshade");

    m_terrainVao.create();
    m_terrainVao.bind();

    m_terrainVerts = m_terrain.generateTerrain();

    m_terrainVbo.create();
    m_terrainVbo.bind();
    glBufferData(GL_ARRAY_BUFFER, m_terrainVerts.size() * sizeof(GLfloat),
                 m_terrainVerts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                          reinterpret_cast<void *>(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                          reinterpret_cast<void *>(6 * sizeof(GLfloat)));

    m_terrainVbo.release();

    m_terrainWorld.setToIdentity();


    m_terrainWorld.rotate(-45.0f, QVector3D(0, 0, 1));
    m_terrainWorld.translate(QVector3D(-0.5,-0.5,0));

    m_terrainCamera.setToIdentity();
    m_terrainCamera.lookAt(QVector3D(0,2,1),QVector3D(0,0,0),QVector3D(0,0,1));

    m_terrainProgram->release();
    rebuildTerrainMatrices();
}

void Realtime::rebuildTerrainMatrices() {
    m_terrainCamera.setToIdentity();
    QMatrix4x4 rot;
    rot.setToIdentity();
    rot.rotate(-10 * m_angleX, QVector3D(0, 0, 1));
    QVector3D eye = QVector3D(1, 1, 1);
    eye = rot.map(eye);
    rot.setToIdentity();
    rot.rotate(-10 * m_angleY, QVector3D::crossProduct(QVector3D(0, 0, 1), eye));
    eye = rot.map(eye);

    eye = eye * m_zoom;

    m_terrainCamera.lookAt(eye, QVector3D(0, 0, 0), QVector3D(0, 0, 1));

    m_terrainProj.setToIdentity();
    m_terrainProj.perspective(45.0f, 1.0 * width() / height(), 0.01f, 100.0f);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m_terrainViewMatrix[i][j] = m_terrainCamera(j, i);
            m_terrainProjMatrix[i][j] = m_terrainProj(j, i);
            m_terrainWorldMatrix[i][j] = m_terrainWorld(j, i);
        }
    }
}

void Realtime::updateAffectedTiles(const std::unordered_set<int>& affectedTiles) {
    if (affectedTiles.empty()) return;

    for (int tileIndex : affectedTiles) {
        int tilesPerSide = m_terrain.getTilesPerSide();
        int tileX = tileIndex % tilesPerSide;
        int tileY = tileIndex / tilesPerSide;

        m_terrain.updateTile(tileX, tileY, m_terrainVerts);
    }

    m_terrainVbo.bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_terrainVerts.size() * sizeof(GLfloat),
                    m_terrainVerts.data());
    m_terrainVbo.release();
}

void Realtime::updateAffectedTiles(float x, float y, float radius) {
    std::vector<int> affectedTiles = m_terrain.getAffectedTiles(x, y, radius);

    if (affectedTiles.empty()) return;

    for (int tileIndex : affectedTiles) {
        int tilesPerSide = m_terrain.getTilesPerSide();
        int tileX = tileIndex % tilesPerSide;
        int tileY = tileIndex / tilesPerSide;

        m_terrain.updateTile(tileX, tileY, m_terrainVerts);
    }

    m_terrainVbo.bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_terrainVerts.size() * sizeof(GLfloat),
                    m_terrainVerts.data());
    m_terrainVbo.release();
}

// ========== SARYA: TERRAIN OBJECT PLACEMENT SYSTEM - REPLACE THIS CODE AS NEEDED ==========

void Realtime::placeObjectOnTerrain(float terrainX, float terrainY, PrimitiveType type, float size) {
    std::vector<float> vertexData = getVertexDataForType(type);

    if (vertexData.empty()) {
        std::cerr << "Error: Empty vertex data for object type " << static_cast<int>(type) << std::endl;
        return;
    }

    if (vertexData.size() == 0) {
        std::cerr << "Error: No vertices for object type " << static_cast<int>(type) << std::endl;
        return;
    }

    // Clamp to terrain bounds
    terrainX = glm::clamp(terrainX, 0.0f, 1.0f);
    terrainY = glm::clamp(terrainY, 0.0f, 1.0f);

    // Get terrain height at this position
    float terrainHeight = m_terrain.getHeight(terrainX, terrainY);
    // Create object
    TerrainObject obj;
    obj.type = type;
    obj.terrainPosition = glm::vec2(terrainX, terrainY);
    obj.size = size;

    // Build transformation matrix
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(terrainX, terrainY, terrainHeight));

    // Adjust placement based on object type
    switch(type) {
    case PrimitiveType::PRIMITIVE_CUBE:
        // Center cube on top of terrain
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, size * 0.5f));
        break;
    case PrimitiveType::PRIMITIVE_SPHERE:
        // Center sphere on terrain
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, size * 0.5f));
        break;
    case PrimitiveType::PRIMITIVE_CONE:
        // Base of cone sits on terrain
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        break;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        // Center cylinder on terrain
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, size * 0.5f));
        break;
    default:
        break;
    }

    modelMatrix = glm::scale(modelMatrix, glm::vec3(size));
    obj.modelMatrix = modelMatrix;

    // Random color for variety
    obj.color = glm::vec4(
        0.3f + (rand() % 70) / 100.0f,
        0.3f + (rand() % 70) / 100.0f,
        0.3f + (rand() % 70) / 100.0f,
        1.0f
        );

    // Get appropriate vertex data based on type
  //  std::vector<float> vertexData = getVertexDataForType(type);

    if (vertexData.empty()) {
        std::cerr << "Failed to get vertex data for object type" << std::endl;
        return;
    }

    // Create OpenGL buffers
    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat),
                 vertexData.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                          reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                          reinterpret_cast<void *>(sizeof(GLfloat)*3));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (vbo == 0 || vao == 0) {
        std::cerr << "Error: Failed to create OpenGL buffers" << std::endl;
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);
        return;
    }

    obj.vbo = vbo;
    obj.vao = vao;
    obj.vertexCount = vertexData.size() / 6;

    if (obj.vertexCount <= 0) {
        std::cerr << "Error: Invalid vertex count: " << obj.vertexCount << std::endl;
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        return;
    }

    m_terrainObjects.push_back(obj);

    // debugging print! shouldn't need it anymore.
   // std::cout << "Placed " << getObjectTypeName(type) << " at terrain ("
   //           << terrainX << ", " << terrainY << "), height: " << terrainHeight << std::endl;

    if (m_particles) {
        glm::vec3 worldPos = glm::vec3(m_terrainWorldMatrix * glm::vec4(terrainX, terrainY, terrainHeight, 1.0f));
        triggerParticleBurst(worldPos.x, worldPos.y, worldPos.z);
    }


    update();
}

std::vector<float> Realtime::getVertexDataForType(PrimitiveType type) {
    switch(type) {
    case PrimitiveType::PRIMITIVE_CUBE:
        return m_cube_data;
    case PrimitiveType::PRIMITIVE_SPHERE:
        return m_sphere_data;
    case PrimitiveType::PRIMITIVE_CONE:
        return m_cone_data;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        return m_cylinder_data;
    default:
        return std::vector<float>();
    }
}

std::string Realtime::getObjectTypeName(PrimitiveType type) {
    switch(type) {
    case PrimitiveType::PRIMITIVE_CUBE: return "Cube";
    case PrimitiveType::PRIMITIVE_SPHERE: return "Sphere";
    case PrimitiveType::PRIMITIVE_CONE: return "Cone";
    case PrimitiveType::PRIMITIVE_CYLINDER: return "Cylinder";
    default: return "Unknown";
    }
}

void Realtime::clearTerrainObjects() {
    for (const TerrainObject& obj : m_terrainObjects) {
        glDeleteBuffers(1, &obj.vbo);
        glDeleteVertexArrays(1, &obj.vao);
    }
    m_terrainObjects.clear();
    update();
    std::cout << "Cleared all terrain objects" << std::endl;
}

GLuint Realtime::typeInterpretVao(PrimitiveType type) {
    switch (type) {
    case PrimitiveType::PRIMITIVE_CONE:
        return m_cone_vao;
    case PrimitiveType::PRIMITIVE_CUBE:
        return m_cube_vao;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        return m_cylinder_vao;
    case PrimitiveType::PRIMITIVE_SPHERE:
        return m_sphere_vao;
    default:
        return 0;
    }
}

GLsizei Realtime::typeInterpretVertices(PrimitiveType type) {
    switch (type) {
    case PrimitiveType::PRIMITIVE_CONE:
        return m_cone_data.size()/6;
    case PrimitiveType::PRIMITIVE_CUBE:
        return m_cube_data.size()/6;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        return m_cylinder_data.size()/6;
    case PrimitiveType::PRIMITIVE_SPHERE:
        return m_sphere_data.size()/6;
    default:
        return 0;
    }
}

void Realtime::paintGL() {
    // ========== PREPROCESSING FBO WRAP ==========
    glBindFramebuffer(GL_FRAMEBUFFER, m_preprocessFBO);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ========== SHADOW PASS (FIRST) ==========
    glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    m_lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(m_depthShader);

    if (m_depthUniformLocs.lightSpaceMatrix != -1) {
        glUniformMatrix4fv(m_depthUniformLocs.lightSpaceMatrix, 1, GL_FALSE, &m_lightSpaceMatrix[0][0]);
    }

    // Shadow pass for scene shapes
    for (const RenderShapeData& shape : renderData.shapes) {
        if (m_depthUniformLocs.model != -1) {
            glUniformMatrix4fv(m_depthUniformLocs.model, 1, GL_FALSE, &shape.ctm[0][0]);
        }
        glBindVertexArray(typeInterpretVao(shape.primitive.type));
        glDrawArrays(GL_TRIANGLES, 0, typeInterpretVertices(shape.primitive.type));
        glBindVertexArray(0);
    }

    // Shadow pass for terrain objects (skip flags and rocks with special shaders)
    for (const TerrainObject& obj : m_terrainObjects) {
        if (obj.isFlag || obj.isRock) continue;

        if (m_depthUniformLocs.model != -1) {
            glUniformMatrix4fv(m_depthUniformLocs.model, 1, GL_FALSE, &obj.modelMatrix[0][0]);
        }


        glBindVertexArray(obj.vao);
        glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
        glBindVertexArray(0);
    }

    // Shadow pass for base model
    if (m_baseModel_data.size() > 0) {
        glm::mat4 fullModelMatrix = m_terrainWorldMatrix * m_baseModelMatrix;
        if (m_depthUniformLocs.model != -1) {
            glUniformMatrix4fv(m_depthUniformLocs.model, 1, GL_FALSE, &fullModelMatrix[0][0]);
        }
        glBindVertexArray(m_baseModel_vao);
        glDrawArrays(GL_TRIANGLES, 0, m_baseModel_data.size() / 6);
        glBindVertexArray(0);
    }

    // ========== MAIN SCENE RENDER ==========
    glBindFramebuffer(GL_FRAMEBUFFER, m_preprocessFBO);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ========== SKYBOX RENDERING ==========
    glDepthMask(GL_FALSE);

    if (m_showTerrain) {
        glm::mat4 terrainViewMatrix = glm::mat4(1.0f);
        glm::mat4 terrainProjMatrix = glm::mat4(1.0f);

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                terrainViewMatrix[i][j] = m_terrainCamera(j, i);
                terrainProjMatrix[i][j] = m_terrainProj(j, i);
            }
        }

        m_skybox.render(&terrainViewMatrix[0][0], &terrainProjMatrix[0][0]);
    } else {
        m_skybox.render(&m_view[0][0], &m_proj[0][0]);
    }

    glDepthMask(GL_TRUE);

    // ========== TERRAIN RENDERING ==========
    if (m_showTerrain) {
        glEnable(GL_DEPTH_TEST);

        m_terrainProgram->bind();
        m_terrainProgram->setUniformValue(m_terrainProjMatrixLoc, m_terrainProj);
        m_terrainProgram->setUniformValue(m_terrainMvMatrixLoc, m_terrainCamera * m_terrainWorld);
        m_terrainProgram->setUniformValue(m_terrainWireshadeLoc, m_terrain.m_wireshade);

        int res = m_terrain.getResolution();

        m_terrainVao.bind();
        glPolygonMode(GL_FRONT_AND_BACK, m_terrain.m_wireshade ? GL_LINE : GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, res * res * 6);
        m_terrainVao.release();

        m_terrainProgram->release();
    }

    // ========== BASE MODEL RENDERING (UNDER TERRAIN) ==========
    if (m_baseModel_data.size() > 0) {
        glUseProgram(m_shader);

        // Convert terrain camera matrices
        glm::mat4 terrainViewMatrix = glm::mat4(1.0f);
        glm::mat4 terrainProjMatrix = glm::mat4(1.0f);

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                terrainViewMatrix[i][j] = m_terrainCamera(j, i);
                terrainProjMatrix[i][j] = m_terrainProj(j, i);
            }
        }

        // Set uniforms
        if (m_uniformLocs.view != -1) glUniformMatrix4fv(m_uniformLocs.view, 1, GL_FALSE, &terrainViewMatrix[0][0]);
        if (m_uniformLocs.projection != -1) glUniformMatrix4fv(m_uniformLocs.projection, 1, GL_FALSE, &terrainProjMatrix[0][0]);
        if (m_uniformLocs.lightSpaceMatrix != -1) glUniformMatrix4fv(m_uniformLocs.lightSpaceMatrix, 1, GL_FALSE, &m_lightSpaceMatrix[0][0]);
        if (m_uniformLocs.lightPos != -1) glUniform3fv(m_uniformLocs.lightPos, 1, &lightPos[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_depthMap);
        if (m_uniformLocs.shadowMap != -1) glUniform1i(m_uniformLocs.shadowMap, 0);

        glBindVertexArray(m_baseModel_vao);

        glm::mat4 fullModelMatrix = m_terrainWorldMatrix * m_baseModelMatrix;

        if (m_uniformLocs.model != -1) glUniformMatrix4fv(m_uniformLocs.model, 1, GL_FALSE, &fullModelMatrix[0][0]);

        glm::mat4 mvp = terrainProjMatrix * terrainViewMatrix * fullModelMatrix;
        if (m_uniformLocs.mvp != -1) glUniformMatrix4fv(m_uniformLocs.mvp, 1, GL_FALSE, &mvp[0][0]);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(fullModelMatrix)));
        if (m_uniformLocs.ictm != -1) glUniformMatrix3fv(m_uniformLocs.ictm, 1, GL_FALSE, &normalMatrix[0][0]);

        // Set base model color (brown/rocky)
        glm::vec4 baseColor = glm::vec4(0.4f, 0.3f, 0.2f, 1.0f);
        if (m_uniformLocs.cAmbient != -1) glUniform4fv(m_uniformLocs.cAmbient, 1, &baseColor[0]);
        if (m_uniformLocs.cDiffuse != -1) glUniform4fv(m_uniformLocs.cDiffuse, 1, &baseColor[0]);
        if (m_uniformLocs.cSpecular != -1) glUniform4f(m_uniformLocs.cSpecular, 0.2f, 0.2f, 0.2f, 1.0f);
        if (m_uniformLocs.shininess != -1) glUniform1f(m_uniformLocs.shininess, 16.0f);
        if (m_uniformLocs.ka != -1) glUniform1f(m_uniformLocs.ka, renderData.globalData.ka);
        if (m_uniformLocs.kd != -1) glUniform1f(m_uniformLocs.kd, 0.8f);
        if (m_uniformLocs.ks != -1) glUniform1f(m_uniformLocs.ks, 0.5f);

        glDrawArrays(GL_TRIANGLES, 0, m_baseModel_data.size() / 6);
        glBindVertexArray(0);
    }

    // ========== REGULAR TERRAIN OBJECTS RENDERING ==========
    glUseProgram(m_shader);

    glm::mat4 terrainViewMatrix = glm::mat4(1.0f);
    glm::mat4 terrainProjMatrix = glm::mat4(1.0f);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            terrainViewMatrix[i][j] = m_terrainCamera(j, i);
            terrainProjMatrix[i][j] = m_terrainProj(j, i);
        }
    }

    glm::mat4 terrainMVMatrix = terrainViewMatrix * m_terrainWorldMatrix;

    if (m_uniformLocs.view != -1) glUniformMatrix4fv(m_uniformLocs.view, 1, GL_FALSE, &terrainViewMatrix[0][0]);
    if (m_uniformLocs.projection != -1) glUniformMatrix4fv(m_uniformLocs.projection, 1, GL_FALSE, &terrainProjMatrix[0][0]);
    if (m_uniformLocs.lightSpaceMatrix != -1) glUniformMatrix4fv(m_uniformLocs.lightSpaceMatrix, 1, GL_FALSE, &m_lightSpaceMatrix[0][0]);
    if (m_uniformLocs.lightPos != -1) glUniform3fv(m_uniformLocs.lightPos, 1, &lightPos[0]);
    if (m_uniformLocs.cameraPos != -1) {
        QVector3D eye = m_terrainCamera * QVector3D(0, 0, 0);
        glm::vec4 camPosVec(eye.x(), eye.y(), eye.z(), 1.0f);
        glUniform4fv(m_uniformLocs.cameraPos, 1, &camPosVec[0]);
    }
    if (m_uniformLocs.ka != -1) glUniform1f(m_uniformLocs.ka, renderData.globalData.ka);
    if (m_uniformLocs.kd != -1) glUniform1f(m_uniformLocs.kd, 0.8f);
    if (m_uniformLocs.ks != -1) glUniform1f(m_uniformLocs.ks, 0.5f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    if (m_uniformLocs.shadowMap != -1) {
        glUniform1i(m_uniformLocs.shadowMap, 0);
    }

    // Render regular terrain objects (not rocks or flags)
    for (const TerrainObject& obj : m_terrainObjects) {
        if (obj.isRock || obj.isFlag) continue;

        glBindVertexArray(obj.vao);

        glm::mat4 fullModelMatrix = m_terrainWorldMatrix * obj.modelMatrix;

        if (m_uniformLocs.model != -1) glUniformMatrix4fv(m_uniformLocs.model, 1, GL_FALSE, &fullModelMatrix[0][0]);

        glm::mat4 mvp = terrainProjMatrix * terrainViewMatrix * fullModelMatrix;
        if (m_uniformLocs.mvp != -1) glUniformMatrix4fv(m_uniformLocs.mvp, 1, GL_FALSE, &mvp[0][0]);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(fullModelMatrix)));
        if (m_uniformLocs.ictm != -1) glUniformMatrix3fv(m_uniformLocs.ictm, 1, GL_FALSE, &normalMatrix[0][0]);

        if (m_uniformLocs.cAmbient != -1) glUniform4fv(m_uniformLocs.cAmbient, 1, &obj.color[0]);
        if (m_uniformLocs.cDiffuse != -1) glUniform4fv(m_uniformLocs.cDiffuse, 1, &obj.color[0]);
        if (m_uniformLocs.cSpecular != -1) glUniform4f(m_uniformLocs.cSpecular, 0.5f, 0.5f, 0.5f, 1.0f);
        if (m_uniformLocs.shininess != -1) glUniform1f(m_uniformLocs.shininess, 32.0f);
        if (m_uniformLocs.cReflective != -1) glUniform4f(m_uniformLocs.cReflective, 0.0f, 0.0f, 0.0f, 0.0f);

        glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
        glBindVertexArray(0);
    }

    // ========== FLAG RENDERING ==========
    glDisable(GL_CULL_FACE);  // Flags need to be visible from both sides

    for (const TerrainObject& obj : m_terrainObjects) {
        if (!obj.isFlag || !obj.flagSimulation) continue;

        glm::mat4 fullModelMatrix = m_terrainWorldMatrix * obj.modelMatrix;

        if (m_uniformLocs.model != -1) glUniformMatrix4fv(m_uniformLocs.model, 1, GL_FALSE, &fullModelMatrix[0][0]);

        glm::mat4 mvp = terrainProjMatrix * terrainViewMatrix * fullModelMatrix;
        if (m_uniformLocs.mvp != -1) glUniformMatrix4fv(m_uniformLocs.mvp, 1, GL_FALSE, &mvp[0][0]);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(fullModelMatrix)));
        if (m_uniformLocs.ictm != -1) glUniformMatrix3fv(m_uniformLocs.ictm, 1, GL_FALSE, &normalMatrix[0][0]);

        // Render pole with brown color
        glm::vec4 poleColor = glm::vec4(0.6f, 0.4f, 0.2f, 1.0f);
        if (m_uniformLocs.cAmbient != -1) glUniform4fv(m_uniformLocs.cAmbient, 1, &poleColor[0]);
        if (m_uniformLocs.cDiffuse != -1) glUniform4fv(m_uniformLocs.cDiffuse, 1, &poleColor[0]);
        if (m_uniformLocs.cSpecular != -1) glUniform4f(m_uniformLocs.cSpecular, 0.3f, 0.3f, 0.3f, 1.0f);
        if (m_uniformLocs.shininess != -1) glUniform1f(m_uniformLocs.shininess, 32.0f);

        obj.flagSimulation->renderPole();

        // Render flag cloth with flag color
        if (m_uniformLocs.cAmbient != -1) glUniform4fv(m_uniformLocs.cAmbient, 1, &obj.color[0]);
        if (m_uniformLocs.cDiffuse != -1) glUniform4fv(m_uniformLocs.cDiffuse, 1, &obj.color[0]);
        if (m_uniformLocs.cSpecular != -1) glUniform4f(m_uniformLocs.cSpecular, 0.5f, 0.5f, 0.5f, 1.0f);
        if (m_uniformLocs.shininess != -1) glUniform1f(m_uniformLocs.shininess, 32.0f);

        obj.flagSimulation->render();
    }

    glEnable(GL_CULL_FACE);

    // ========== ROCK RENDERING WITH BUMP MAPPING ==========
    if (m_bumpMapping.isInitialized()) {
        GLuint bumpShader = m_bumpMapping.getShader();


        if (bumpShader != 0) {
            glUseProgram(bumpShader);

            GLint uModelMatrix = glGetUniformLocation(bumpShader, "modelMatrix");
            GLint uViewMatrix = glGetUniformLocation(bumpShader, "viewMatrix");
            GLint uProjMatrix = glGetUniformLocation(bumpShader, "projMatrix");
            GLint uLightDir = glGetUniformLocation(bumpShader, "lightDir");
            GLint uMatColor = glGetUniformLocation(bumpShader, "matColor");
            GLint uBumpPattern = glGetUniformLocation(bumpShader, "bumpPattern");
            GLint uBumpScale = glGetUniformLocation(bumpShader, "bumpScale");
            GLint uBumpVariation = glGetUniformLocation(bumpShader, "bumpVariation");
            GLint uBumpSeed = glGetUniformLocation(bumpShader, "bumpSeed");
            GLint uBumpStrength = glGetUniformLocation(bumpShader, "bumpStrength");

            if (uViewMatrix != -1) glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, &terrainViewMatrix[0][0]);
            if (uProjMatrix != -1) glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, &terrainProjMatrix[0][0]);

            glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));
            if (uLightDir != -1) glUniform3fv(uLightDir, 1, &lightDir[0]);

            if (uBumpPattern != -1) glUniform1i(uBumpPattern, 0);
            if (uBumpScale != -1) glUniform1f(uBumpScale, m_bumpMapping.getBumpScale());
            if (uBumpVariation != -1) glUniform1f(uBumpVariation, m_bumpMapping.getBumpVariation());
            if (uBumpSeed != -1) glUniform1f(uBumpSeed, m_bumpMapping.getBumpSeed());
            if (uBumpStrength != -1) glUniform1f(uBumpStrength, m_bumpMapping.getBumpStrength());

            for (const TerrainObject& obj : m_terrainObjects) {
                if (!obj.isRock) continue;

                glBindVertexArray(obj.vao);

                glm::mat4 fullModelMatrix = m_terrainWorldMatrix * obj.modelMatrix;

                if (uModelMatrix != -1) glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, &fullModelMatrix[0][0]);

                if (uMatColor != -1) {
                    glm::vec3 color3(obj.color.r, obj.color.g, obj.color.b);
                    glUniform3fv(uMatColor, 1, &color3[0]);
                }

                glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
                glBindVertexArray(0);
            }
        }
    }

    if (m_showParticles && m_particles) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        // Convert terrain matrices to glm::mat4
        glm::mat4 terrainViewMatrix = glm::mat4(1.0f);
        glm::mat4 terrainProjMatrix = glm::mat4(1.0f);

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                terrainViewMatrix[i][j] = m_terrainCamera(j, i);
                terrainProjMatrix[i][j] = m_terrainProj(j, i);
            }
        }

        m_particles->setTerrainMatrices(terrainViewMatrix, terrainProjMatrix, m_terrainWorldMatrix);
        m_particles->paintGL();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_preprocessFBO);

    //copy to default
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    glBlitFramebuffer(
        0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio,
        0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
        );

    // restore
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Realtime::resizeGL(int w, int h) {
    glBindTexture(GL_TEXTURE_2D, m_preprocessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 w * m_devicePixelRatio, h * m_devicePixelRatio,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, m_preprocessDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          w * m_devicePixelRatio, h * m_devicePixelRatio);
}

void Realtime::updateCamera() {
    m_view = camera.calculateViewMatrix(camLook, camUp, camPos);
    m_proj = camera.calculateProjectionMatrix(settings.nearPlane, settings.farPlane);
}

void Realtime::updateLights() {
    SceneGlobalData gd = renderData.globalData;
    m_ka = gd.ka;
    m_kd = gd.kd;
    m_ks = gd.ks;

    glUseProgram(m_shader);

    std::vector<SceneLightData> lights = renderData.lights;
    GLint numLightsLoc = glGetUniformLocation(m_shader, "numLights");
    if (numLightsLoc != -1) {
        glUniform1i(numLightsLoc, std::min(8, static_cast<int>(lights.size())));
    }

    for (size_t i = 0; i < lights.size() && i < 8; i++) {
        SceneLightData light = lights[i];
        std::string base = "lights[" + std::to_string(i) + "]";

        LightType type = light.type;
        int typeInt = type == LightType::LIGHT_POINT ? 0 : (type == LightType::LIGHT_DIRECTIONAL ? 1 : 2);

        GLint lightTypeLoc = glGetUniformLocation(m_shader, (base + ".type").c_str());
        if (lightTypeLoc != -1) glUniform1i(lightTypeLoc, typeInt);

        GLint lightColorLoc = glGetUniformLocation(m_shader, (base + ".color").c_str());
        if (lightColorLoc != -1) glUniform4fv(lightColorLoc, 1, &light.color[0]);

        GLint lightFunctionLoc = glGetUniformLocation(m_shader, (base + ".function").c_str());
        if (lightFunctionLoc != -1) glUniform3fv(lightFunctionLoc, 1, &light.function[0]);

        GLint lightPosLoc = glGetUniformLocation(m_shader, (base + ".pos").c_str());
        if (lightPosLoc != -1) glUniform4fv(lightPosLoc, 1, &light.pos[0]);

        GLint lightDirLoc = glGetUniformLocation(m_shader, (base + ".dir").c_str());
        if (lightDirLoc != -1) glUniform4fv(lightDirLoc, 1, &light.dir[0]);

        GLint lightPenumbraLoc = glGetUniformLocation(m_shader, (base + ".penumbra").c_str());
        if (lightPenumbraLoc != -1) glUniform1f(lightPenumbraLoc, light.penumbra);

        GLint lightAngleLoc = glGetUniformLocation(m_shader, (base + ".angle").c_str());
        if (lightAngleLoc != -1) glUniform1f(lightAngleLoc, light.angle);
    }
    glUseProgram(0);
}

void Realtime::sceneChanged() {
    renderData.lights.clear();
    bool success = SceneParser::parse(settings.sceneFilePath, renderData);
    if (!success) return;

    SceneCameraData camData = renderData.cameraData;
    camera.cameraSetUp(camData, size().width(), size().height());
    camLook = glm::normalize(glm::vec3(camData.look));
    camUp = glm::normalize(glm::vec3(camData.up));
    camPos = camData.pos;
    updateCamera();
    updateLights();

    update();
}

void Realtime::settingsChanged() {
    if (isSetUp) updateShapes();
    update();
}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;

    if (event->key() == Qt::Key_T) {
        m_showTerrain = !m_showTerrain;
        std::cout << "Terrain " << (m_showTerrain ? "enabled" : "disabled") << std::endl;
        update();
    }

    // Toggle object placement mode
    if (event->key() == Qt::Key_C) {
        m_placeObjectMode = !m_placeObjectMode;
        if (m_placeObjectMode) {
            std::cout << "Object placement mode enabled - Click on terrain to place objects" << std::endl;
        } else {
            std::cout << "Object placement mode disabled" << std::endl;
        }
    }

    // Switch to Primitive mode
    if (event->key() == Qt::Key_P) {
        m_placementMode = PlacementMode::PRIMITIVE;
        std::cout << "primitive type" << std::endl;

    }

    // Switch to L-SYSTEM mode
    if (event->key() == Qt::Key_L) {
        m_placementMode = PlacementMode::LSYSTEM;
        std::cout << "l-system type" << std::endl;
    }

    //Switch to rock generation
    if (event->key() == Qt::Key_R) {
        m_placementMode = PlacementMode::ROCK;
        std::cout << "Rock Placement" << std::endl;
    }

    //Flag generation
    if (event->key() == Qt::Key_F) {
        m_placementMode = PlacementMode::FLAG;
        std::cout << "Flag Placement" << std::endl;
    }

    if (event->key() == Qt::Key_1) {
        if (m_placementMode == PlacementMode::PRIMITIVE) {
            m_currentObjectType = PrimitiveType::PRIMITIVE_CUBE;
            std::cout << "Selected: Cube" << std::endl;
        } else {
            m_currentTreePreset = "Dense";
            std::cout << "Selected: Dense tree" << std::endl;
        }
    }
    if (event->key() == Qt::Key_2) {
        if (m_placementMode == PlacementMode::PRIMITIVE) {
            m_currentObjectType = PrimitiveType::PRIMITIVE_SPHERE;
            std::cout << "Selected: Sphere" << std::endl;
        } else {
            m_currentTreePreset = "Simple";
            std::cout << "Selected: Simple tree" << std::endl;
        }
    }
    if (event->key() == Qt::Key_3) {
        if (m_placementMode == PlacementMode::PRIMITIVE) {
            m_currentObjectType = PrimitiveType::PRIMITIVE_CONE;
            std::cout << "Selected: Cone" << std::endl;
        } else {
            m_currentTreePreset = "Bush";
            std::cout << "Selected: Bush" << std::endl;
        }
    }
    if (event->key() == Qt::Key_4) {
        if (m_placementMode == PlacementMode::PRIMITIVE) {
            m_currentObjectType = PrimitiveType::PRIMITIVE_CYLINDER;
            std::cout << "Selected: Cylinder" << std::endl;
        } else {
            m_currentTreePreset = "Asymmetric";
            std::cout << "Selected: Asymmetric tree" << std::endl;
        }
    }

    // Clear all terrain objects
    if (event->key() == Qt::Key_X) {
        clearTerrainObjects();
    }

    if (event->key() == Qt::Key_V) {
        if (!m_particles) {
            m_particles = new Particles(this);
            m_particles->initializeGL();
        }
        m_showParticles = !m_showParticles;
        std::cout << "Particles " << (m_showParticles ? "enabled" : "disabled") << std::endl;
    }
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }

    // Terrain interaction with left button
    if (m_showTerrain && event->buttons().testFlag(Qt::LeftButton)) {
        m_prevMousePosQt = event->pos();

        std::optional<glm::vec3> planeInt = mouse::mouse_click_callback(
            1, 1, event->pos().x(), event->pos().y(),
            m_w, m_h, m_terrainProjMatrix, m_terrainViewMatrix, m_terrainVerts,
            m_terrain.getResolution(), m_terrainWorldMatrix);

        if (planeInt.has_value()) {
            m_intersected = 1;
            glm::vec3 hitpoint = planeInt.value();
            glm::mat4 worldInverse = glm::inverse(m_terrainWorldMatrix);
            m_hitPoint = glm::vec3(worldInverse * glm::vec4(hitpoint, 1.0f));

            if (m_placeObjectMode) {
                if (m_placementMode == PlacementMode::PRIMITIVE) {
                    placeObjectOnTerrain(m_hitPoint.x, m_hitPoint.y,
                                         m_currentObjectType, 0.05f);
                } else if (m_placementMode == PlacementMode::LSYSTEM) {
                    placeLSystemOnTerrain(m_hitPoint.x, m_hitPoint.y,
                                          m_currentTreePreset,
                                          3, 0.1f);
                } else if (m_placementMode == PlacementMode::ROCK) {
                    placeRockOnTerrain(m_hitPoint.x, m_hitPoint.y, 0.03f);
                }else if (m_placementMode == PlacementMode::FLAG) {
                    placeFlagOnTerrain(m_hitPoint.x, m_hitPoint.y, 0.05f);
                }
            }
            // Terrain sculpting mode
            else {
                float craterDepth = 0.005f;
                float craterRadius = 0.01f;

                std::vector<std::pair<float, float>> offsets = {
                    {0.075f, 0.0f},
                    {-0.075f, 0.0f},
                    {0.025f, 0.00f},
                    {-0.025f, 0.0f}
                };

                std::unordered_set<int> allAffectedTiles;

                for (const auto& offset : offsets) {
                    float x = m_hitPoint.x + offset.first;
                    float y = m_hitPoint.y + offset.second;

                    if (x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f) {
                        m_terrain.divot(x, y, craterDepth, craterRadius);
                        std::vector<int> tiles = m_terrain.getAffectedTiles(x, y, craterRadius);
                        allAffectedTiles.insert(tiles.begin(), tiles.end());
                    }
                }

                float totalRadius = craterRadius + 0.05f;
                std::vector<int> totalTiles = m_terrain.getAffectedTiles(m_hitPoint.x, m_hitPoint.y, totalRadius);
                allAffectedTiles.insert(totalTiles.begin(), totalTiles.end());

                updateAffectedTiles(allAffectedTiles);
            }
        }
        else {
            m_intersected = 0;
        }
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
    m_intersected = 0;
}

glm::mat3 rotMat(glm::vec3 &u, float angle) {
    return glm::mat3(cos(angle)+u.x*u.x*(1-cos(angle)),
                     u.x*u.y*(1-cos(angle))+u.z*sin(angle),
                     u.x*u.z*(1-cos(angle))-u.y*sin(angle),
                     u.x*u.y*(1-cos(angle))-u.z*sin(angle),
                     cos(angle)+u.y*u.y*(1-cos(angle)),
                     u.y*u.z*(1-cos(angle))+u.x*sin(angle),
                     u.x*u.z*(1-cos(angle))+u.y*sin(angle),
                     u.y*u.z*(1-cos(angle))-u.x*sin(angle),
                     cos(angle)+u.z*u.z*(1-cos(angle)));
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    // Terrain sculpting when dragging
    if (m_mouseDown && m_showTerrain && m_intersected == 1 && !m_placeObjectMode) {
        std::optional<glm::vec3> planeInt = mouse::mouse_click_callback(
            1, 1, event->pos().x(), event->pos().y(),
            m_w, m_h, m_terrainProjMatrix, m_terrainViewMatrix, m_terrainVerts,
            m_terrain.getResolution(), m_terrainWorldMatrix);

        if (planeInt.has_value()) {
            glm::vec3 hitpoint = planeInt.value();
            glm::mat4 worldInverse = glm::inverse(m_terrainWorldMatrix);
            glm::vec3 newHitPoint = glm::vec3(worldInverse * glm::vec4(hitpoint, 1.0f));

            float dx = newHitPoint.x - m_hitPoint.x;
            float dy = newHitPoint.y - m_hitPoint.y;
            float distSq = dx * dx + dy * dy;

            if (distSq > 0.00025f) {
                m_hitPoint = newHitPoint;

                float craterDepth = 0.005f;
                float craterRadius = 0.01f;

                std::vector<std::pair<float, float>> offsets = {
                    {0.075f, 0.0f},
                    {-0.075f, 0.0f},
                    {0.025f, 0.00f},
                    {-0.025f, 0.0f}
                };

                std::unordered_set<int> allAffectedTiles;

                for (const auto& offset : offsets) {
                    float x = m_hitPoint.x + offset.first;
                    float y = m_hitPoint.y + offset.second;

                    if (x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f) {
                        m_terrain.divot(x, y, craterDepth, craterRadius);
                        std::vector<int> tiles = m_terrain.getAffectedTiles(x, y, craterRadius);
                        allAffectedTiles.insert(tiles.begin(), tiles.end());
                    }
                }

                float totalRadius = craterRadius + 0.05f;
                std::vector<int> totalTiles = m_terrain.getAffectedTiles(m_hitPoint.x, m_hitPoint.y, totalRadius);
                allAffectedTiles.insert(totalTiles.begin(), totalTiles.end());

                updateAffectedTiles(allAffectedTiles);
            }
        }
        else {
            m_intersected = 3;
        }
    }
    // Terrain camera rotation
    else if (m_mouseDown && m_showTerrain && m_intersected == 0) {
        m_angleX += 10 * (event->position().x() - m_prevMousePosQt.x()) / (float) width();
        m_angleY += 10 * (event->position().y() - m_prevMousePosQt.y()) / (float) height();
        m_prevMousePosQt = event->pos();
        rebuildTerrainMatrices();
    }
    // Normal scene camera rotation
    else if (m_mouseDown && event->buttons().testFlag(Qt::LeftButton) && !m_showTerrain) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        float xAngle = -deltaX * .01f;
        float yAngle = -deltaY * .01f;

        glm::vec3 xAxis(0.0f, 1.0f, 0.0f);
        glm::mat3 xRot = rotMat(xAxis, xAngle);

        glm::vec3 yAxis = glm::normalize(glm::cross(camLook, camUp));
        glm::mat3 yRot = rotMat(yAxis, yAngle);

        camLook = glm::normalize(xRot*yRot*camLook);
        camUp = glm::normalize(yRot*xRot*camUp);

        updateCamera();
    }
}

void Realtime::wheelEvent(QWheelEvent *event) {
    if (m_showTerrain) {
        m_zoom -= event->angleDelta().y() / 100.f;
        rebuildTerrainMatrices();
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    //Timer growth
    bool needsUpdate = false;
    for (TerrainObject& obj : m_terrainObjects) {
        //flag
        if (obj.isFlag && obj.flagSimulation) {
            obj.flagSimulation->update(deltaTime);
        }
        //tree
        if (obj.isLSystem && obj.currentIteration < obj.maxIteration) {
            obj.timeSincePlacement += deltaTime;
            if (obj.timeSincePlacement >= obj.growthInterval) {
                growTree(obj);
                obj.timeSincePlacement = 0.0f;
                needsUpdate = true;
            }
        }
    }

    float velocity = 5.0f * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(camLook, camUp));

    if (m_keyMap[Qt::Key_W]) camPos += glm::vec4(camLook * velocity, 0.0f);
    if (m_keyMap[Qt::Key_A]) camPos -= glm::vec4(right * velocity, 0.0f);
    if (m_keyMap[Qt::Key_S]) camPos -= glm::vec4(camLook * velocity, 0.0f);
    if (m_keyMap[Qt::Key_D]) camPos += glm::vec4(right * velocity, 0.0f);

    if (m_keyMap[Qt::Key_Control]) camPos -= glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * velocity;
    if (m_keyMap[Qt::Key_Space]) camPos += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * velocity;
    updateCamera();

    update();
}

void Realtime::saveViewportImage(std::string filePath) {

    int fixedWidth = 1024;
    int fixedHeight = 768;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored();

    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}

void Realtime::placeLSystemOnTerrain(float terrainX, float terrainY,
                                     const std::string& preset,
                                     int maxIterations,
                                     float size) {
    terrainX = glm::clamp(terrainX, 0.0f, 1.0f);
    terrainY = glm::clamp(terrainY, 0.0f, 1.0f);

    float terrainHeight = m_terrain.getHeight(terrainX, terrainY);

    // Start with iteration 1
    int startIteration = 1;
    m_treeManager.generateTree(preset, startIteration);

    const std::vector<float>& treeVertexData = m_treeManager.getVertexData();
    int vertexCount = m_treeManager.getVertexCount();

    if (treeVertexData.empty() || vertexCount == 0) {
        std::cerr << "ERROR: L-system tree generation failed!" << std::endl;
        return;
    }

    TerrainObject obj;
    obj.isLSystem = true;
    obj.lSystemPreset = preset;
    obj.type = PrimitiveType::PRIMITIVE_CUBE;
    obj.terrainPosition = glm::vec2(terrainX, terrainY);
    obj.size = size;

    // Growth animation settings
    obj.currentIteration = startIteration;
    obj.maxIteration = maxIterations;
    obj.timeSincePlacement = 0.0f;
    obj.growthInterval = 5.0f;  // 10 SECONDS CHANGE

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(terrainX, terrainY, terrainHeight));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(size));
    obj.modelMatrix = modelMatrix;

    //COLOR
    float greenBase = 0.2f + (rand() % 30) / 100.0f;
    float brownTint = 0.15f + (rand() % 15) / 100.0f;
    obj.color = glm::vec4(brownTint, greenBase, brownTint * 0.5f, 1.0f);

    // Create OpenGL buffers
    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, treeVertexData.size() * sizeof(GLfloat),
                 treeVertexData.data(), GL_DYNAMIC_DRAW);  // Changed to DYNAMIC_DRAW

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                          reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                          reinterpret_cast<void *>(sizeof(GLfloat)*3));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    obj.vbo = vbo;
    obj.vao = vao;
    obj.vertexCount = vertexCount;

    m_terrainObjects.push_back(obj);

    std::cout << "Placed L-system tree '" << preset << "' at iteration " << startIteration
              << " (will grow to " << maxIterations << ")" << std::endl;

    update();
}

void Realtime::growTree(TerrainObject& obj) {
    if (!obj.isLSystem || obj.currentIteration >= obj.maxIteration) {
        return;
    }

    // Increment iteration
    obj.currentIteration++;

    // Regenerate tree with new iteration count
    m_treeManager.generateTree(obj.lSystemPreset, obj.currentIteration);

    const std::vector<float>& treeVertexData = m_treeManager.getVertexData();
    int vertexCount = m_treeManager.getVertexCount();

    if (treeVertexData.empty() || vertexCount == 0) {
        std::cerr << "ERROR: Failed to grow tree!" << std::endl;
        return;
    }

    // Update vertex count
    obj.vertexCount = vertexCount;

    // Update VBO with new geometry
    glBindBuffer(GL_ARRAY_BUFFER, obj.vbo);
    glBufferData(GL_ARRAY_BUFFER, treeVertexData.size() * sizeof(GLfloat),
                 treeVertexData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::cout << "Tree '" << obj.lSystemPreset << "' grew to iteration "
              << obj.currentIteration << std::endl;
}

void Realtime::placeRockOnTerrain(float terrainX, float terrainY, float size) {

    terrainX = glm::clamp(terrainX, 0.0f, 1.0f);
    terrainY = glm::clamp(terrainY, 0.0f, 1.0f);


    float terrainHeight = m_terrain.getHeight(terrainX, terrainY);

    int randomDetail = 1 + (rand() % 3);
    m_rockGenerator.generateRock(randomDetail);
    m_rockGenerator.perturbVertices();

    // Use FULL vertex data (9 floats: pos + normal + tangent)
    const std::vector<float>& rockVertexData = m_rockGenerator.getFullVertexData();
    int vertexCount = m_rockGenerator.getVertexCount();

    if (rockVertexData.empty() || vertexCount == 0) {
        std::cerr << "ERROR: Rock generation failed! Vertex data empty." << std::endl;
        return;
    }

    // Calculate actual number of vertices (9 floats per vertex)
    int actualVertexCount = rockVertexData.size() / 9;
    if (actualVertexCount <= 0) {
        std::cerr << "ERROR: Invalid rock vertex count: " << actualVertexCount << std::endl;
        return;
    }

    TerrainObject obj;
    obj.isLSystem = false;
    obj.isRock = true;
    obj.type = PrimitiveType::PRIMITIVE_CUBE;
    obj.terrainPosition = glm::vec2(terrainX, terrainY);
    obj.size = size;

    obj.currentIteration = 0;
    obj.maxIteration = 0;
    obj.timeSincePlacement = 0.0f;
    obj.growthInterval = 0.0f;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(terrainX, terrainY, terrainHeight));

    float randomRotation = (rand() % 360) * (M_PI / 180.0f);
    modelMatrix = glm::rotate(modelMatrix, randomRotation, glm::vec3(0.0f, 0.0f, 1.0f));

    float scaleVariation = 0.8f + (rand() % 40) / 100.0f;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(size * scaleVariation));

    obj.modelMatrix = modelMatrix;

    // Gray rock color
    float grayValue = 0.5f + (rand() % 30) / 100.0f;
    obj.color = glm::vec4(grayValue, grayValue, grayValue, 1.0f);

    // Create OpenGL buffers with 9 floats per vertex
    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, rockVertexData.size() * sizeof(GLfloat),
                 rockVertexData.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao);

    // Position (location 0) - 3 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9,
                          reinterpret_cast<void *>(0));

    // Normal (location 1) - 3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9,
                          reinterpret_cast<void *>(sizeof(GLfloat) * 3));

    // Tangent (location 2) - 3 floats
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9,
                          reinterpret_cast<void *>(sizeof(GLfloat) * 6));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (vbo == 0 || vao == 0) {
        std::cerr << "Error: Failed to create OpenGL buffers for rock" << std::endl;
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);
        return;
    }

    obj.vbo = vbo;
    obj.vao = vao;
    obj.vertexCount = actualVertexCount;  // Store actual vertex count

    m_terrainObjects.push_back(obj);

    std::cout << "Placed rock with detail level " << randomDetail
              << " at terrain (" << terrainX << ", " << terrainY << ")"
              << " vertices: " << obj.vertexCount
              << " total floats: " << rockVertexData.size() << std::endl;
    update();
}

void Realtime::placeFlagOnTerrain(float terrainX, float terrainY, float size) {
    terrainX = glm::clamp(terrainX, 0.0f, 1.0f);
    terrainY = glm::clamp(terrainY, 0.0f, 1.0f);

    float terrainHeight = m_terrain.getHeight(terrainX, terrainY);

    TerrainObject obj;
    obj.isLSystem = false;
    obj.isRock = false;
    obj.isFlag = true;
    obj.type = PrimitiveType::PRIMITIVE_CUBE;
    obj.terrainPosition = glm::vec2(terrainX, terrainY);
    obj.size = size;

    obj.currentIteration = 0;
    obj.maxIteration = 0;
    obj.timeSincePlacement = 0.0f;
    obj.growthInterval = 0.0f;

    // Create flag simulation
    Flag* flag = new Flag();
    glm::vec3 anchorPos = glm::vec3(0.0f, 0.0f, 0.0f);
    flag->initialize(20, 15, 0.01f, anchorPos);  // Smaller spacing: 0.01

    obj.flagSimulation = flag;

    // Transformation matrix - reasonable size
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(terrainX, terrainY, terrainHeight + 0.05f));  // Slight raise
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));  // Normal size: 1.0
    obj.modelMatrix = modelMatrix;

    // Red flag
    obj.color = glm::vec4(0.8f, 0.1f, 0.1f, 1.0f);

    obj.vao = flag->getVAO();
    obj.vbo = flag->getVBO();
    obj.vertexCount = (20-1) * (15-1) * 6;

    m_terrainObjects.push_back(obj);

    std::cout << "Placed flag at terrain (" << terrainX << ", " << terrainY << ")" << std::endl;

    update();
}
