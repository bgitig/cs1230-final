#include "Particles.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

Particles::Particles(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_programID(0)
    , m_vertexShader(0)
    , m_fragmentShader(0)
    , m_billboardVertexBuffer(0)
    , m_particlesPositionBuffer(0)
    , m_particlesColorBuffer(0)
    , m_vao(0)
    , m_lastUsedParticle(0)
    , m_particlesCount(0)
    , m_cameraPosition(0.0f, 5.0f, 10.0f)
    , m_horizontalAngle(3.14f)
    , m_verticalAngle(0.0f)
    , m_radius(10.0f)
{
    m_particlePositionSizeData = new GLfloat[MaxParticles * 4];
    m_particleColorData = new GLubyte[MaxParticles * 4];

    // Initialize all particles as dead
    for(int i = 0; i < MaxParticles; i++) {
        m_particlesContainer[i].life = -1.0f;
        m_particlesContainer[i].cameradistance = -1.0f;
    }

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // Start timer for animation
    connect(&m_timer, &QTimer::timeout, this, &Particles::updateScene);
    m_timer.start(16); // ~60 FPS
}

Particles::~Particles() {
    makeCurrent();

    if(m_billboardVertexBuffer) glDeleteBuffers(1, &m_billboardVertexBuffer);
    if(m_particlesPositionBuffer) glDeleteBuffers(1, &m_particlesPositionBuffer);
    if(m_particlesColorBuffer) glDeleteBuffers(1, &m_particlesColorBuffer);
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
    if(m_programID) glDeleteProgram(m_programID);
    if(m_vertexShader) glDeleteShader(m_vertexShader);
    if(m_fragmentShader) glDeleteShader(m_fragmentShader);

    delete[] m_particlePositionSizeData;
    delete[] m_particleColorData;

    doneCurrent();
}

void Particles::initializeGL() {
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) fprintf(stderr, "Error while initializing GLEW: %s\n", glewGetErrorString(err));
    fprintf(stdout, "Successfully initialized GLEW %s\n", glewGetString(GLEW_VERSION));

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(!loadShaders()) {
        qWarning("Failed to load shaders");
        return;
    }

    // VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // The VBO containing the 4 vertices of the particles
    // Thanks to instancing, they will be shared by all particles.
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        0.5f,  0.5f, 0.0f,
    };

    glGenBuffers(1, &m_billboardVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_billboardVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    glGenBuffers(1, &m_particlesPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particlesPositionBuffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &m_particlesColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particlesColorBuffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    glBindVertexArray(0);
}

bool Particles::loadShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core

        layout(location = 0) in vec3 squareVertices;
        layout(location = 1) in vec4 xyzs; // Position of the center of the particule and size
        layout(location = 2) in vec4 color;

        out vec2 UV;
        out vec4 particlecolor;

        uniform vec3 CameraRight_worldspace;
        uniform vec3 CameraUp_worldspace;
        uniform mat4 VP;

        void main()
        {
            float particleSize = xyzs.w;
            vec3 particleCenter_worldspace = xyzs.xyz;

            vec3 vertexPosition_worldspace =
                particleCenter_worldspace
                + CameraRight_worldspace * squareVertices.x * particleSize
                + CameraUp_worldspace * squareVertices.y * particleSize;

            gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);

            UV = squareVertices.xy + vec2(0.5, 0.5);
            particlecolor = color;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core

        in vec2 UV;
        in vec4 particlecolor;

        out vec4 color;

        void main()
        {
            // Simple circular particle
            float dist = length(UV - vec2(0.5, 0.5));
            if(dist > 0.5) discard;

            float alpha = 1.0 - (dist / 0.5);
            alpha = alpha * alpha; // Smooth falloff

            color = vec4(particlecolor.rgb, particlecolor.a * alpha);
        }
    )";

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(m_vertexShader);

    GLint success;
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(m_vertexShader, 512, NULL, infoLog);
        qWarning("Vertex shader compilation failed: %s", infoLog);
        return false;
    }

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(m_fragmentShader);

    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);

    m_programID = glCreateProgram();
    glAttachShader(m_programID, m_vertexShader);
    glAttachShader(m_programID, m_fragmentShader);
    glLinkProgram(m_programID);

    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, NULL, infoLog);
        qWarning("Shader program linking failed: %s", infoLog);
        return false;
    }

    m_viewProjMatrixID = glGetUniformLocation(m_programID, "VP");
    m_cameraRightWorldspaceID = glGetUniformLocation(m_programID, "CameraRight_worldspace");
    m_cameraUpWorldspaceID = glGetUniformLocation(m_programID, "CameraUp_worldspace");

    return true;
}

void Particles::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
}

void Particles::paintGL() {
    if(m_particlesCount == 0) return;

    glUseProgram(m_programID);

    // Use external matrices if available, otherwise use internal
    glm::mat4 viewMatrix = m_useExternalMatrices ? m_externalViewMatrix : m_viewMatrix;
    glm::mat4 projectionMatrix = m_useExternalMatrices ? m_externalProjMatrix : m_projectionMatrix;

    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    glm::vec3 cameraRight = glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    glm::vec3 cameraUp = glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

    glUniformMatrix4fv(m_viewProjMatrixID, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform3fv(m_cameraRightWorldspaceID, 1, glm::value_ptr(cameraRight));
    glUniform3fv(m_cameraUpWorldspaceID, 1, glm::value_ptr(cameraUp));

    // VAO
    glBindVertexArray(m_vao);

    // 1st attribute buffer: vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_billboardVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 2nd attribute buffer: particles' centers
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_particlesPositionBuffer);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 3rd attribute buffer: particles' colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_particlesColorBuffer);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    // draw the particles
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_particlesCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Particles::updateScene() {

    double delta = 0.01f;

    m_cameraPosition.x = m_radius * sin(m_verticalAngle) * cos(m_horizontalAngle);
    m_cameraPosition.y = m_radius * cos(m_verticalAngle);
    m_cameraPosition.z = m_radius * sin(m_verticalAngle) * sin(m_horizontalAngle);

    m_viewMatrix = glm::lookAt(
        m_cameraPosition,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
        );


    updateParticles(delta);

    sortParticles();

    // Update the buffers that OpenGL uses for rendering
    glBindBuffer(GL_ARRAY_BUFFER, m_particlesPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particlesCount * sizeof(GLfloat) * 4, m_particlePositionSizeData);

    glBindBuffer(GL_ARRAY_BUFFER, m_particlesColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particlesCount * sizeof(GLubyte) * 4, m_particleColorData);

    update();
}

int Particles::findUnusedParticle() {
    for(int i = m_lastUsedParticle; i < MaxParticles; i++) {
        if(m_particlesContainer[i].life < 0) {
            m_lastUsedParticle = i;
            return i;
        }
    }

    for(int i = 0; i < m_lastUsedParticle; i++) {
        if(m_particlesContainer[i].life < 0) {
            m_lastUsedParticle = i;
            return i;
        }
    }

    return 0; // All particles are taken, override the first one
}

void Particles::sortParticles() {
    std::sort(&m_particlesContainer[0], &m_particlesContainer[MaxParticles]);
}

void Particles::updateParticles(double delta) {
    m_particlesCount = 0;

    // Use external camera position if available
    glm::vec3 cameraPos = m_useExternalMatrices ?
                              glm::vec3(glm::inverse(m_externalViewMatrix)[3]) : m_cameraPosition;

    for(int i = 0; i < MaxParticles; i++) {
        ParticleBurst& p = m_particlesContainer[i];

        if(p.life > 0.0f) {
            p.life -= delta;

            if(p.life > 0.0f) {
                // Simulate simple physics: gravity only, no collisions
                p.speed += glm::vec3(0.0f, 0.0f, -9.81f) * (float)delta * 0.5f; // Note: Z-axis for terrain
                p.pos += p.speed * (float)delta;
                p.cameradistance = glm::length(p.pos - cameraPos);

                // Fill the GPU buffer
                m_particlePositionSizeData[4 * m_particlesCount + 0] = p.pos.x;
                m_particlePositionSizeData[4 * m_particlesCount + 1] = p.pos.y;
                m_particlePositionSizeData[4 * m_particlesCount + 2] = p.pos.z;
                m_particlePositionSizeData[4 * m_particlesCount + 3] = p.size;

                m_particleColorData[4 * m_particlesCount + 0] = p.color[0];
                m_particleColorData[4 * m_particlesCount + 1] = p.color[1];
                m_particleColorData[4 * m_particlesCount + 2] = p.color[2];
                m_particleColorData[4 * m_particlesCount + 3] = p.color[3];
            } else {
                p.cameradistance = -1.0f;
            }

            m_particlesCount++;
        }
    }
}

void Particles::mouseMoveEvent(QMouseEvent *event) {
    static QPoint lastPos = event->pos();

    if(event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - lastPos;
        m_horizontalAngle += delta.x() * 0.01f;
        m_verticalAngle += delta.y() * 0.01f;

        if(m_verticalAngle < 0.1f) m_verticalAngle = 0.1f;
        if(m_verticalAngle > 3.04f) m_verticalAngle = 3.04f;
    }

    lastPos = event->pos();
}

void Particles::wheelEvent(QWheelEvent *event) {
    m_radius -= event->angleDelta().y() * 0.01f;
    if(m_radius < 2.0f) m_radius = 2.0f;
    if(m_radius > 50.0f) m_radius = 50.0f;
}

void Particles::setTerrainMatrices(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& world) {
    m_externalViewMatrix = view;
    m_externalProjMatrix = proj;
    m_externalWorldMatrix = world;
    m_useExternalMatrices = true;
}

void Particles::triggerBurst(const glm::vec3& position) {
    // Create burst of particles at specific position
    int burstCount = 30; // Reduced from 50 - smaller burst

    for(int i = 0; i < burstCount; i++) {
        int particleIndex = findUnusedParticle();
        ParticleBurst& p = m_particlesContainer[particleIndex];

        p.life = 0.5f; // Changed from 2.0f - only lasts 1 second
        p.pos = position;

        // Straight up with slight random variation
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float horizontalSpread = 0.1f + (rand() % 100) / 1000.0f; // Very small horizontal spread
        float upwardSpeed = 0.5f + (rand() % 100) / 1000.0f; // Mostly upward

        glm::vec3 direction = glm::vec3(
            horizontalSpread * cos(angle), // Small X movement
            horizontalSpread * sin(angle), // Small Y movement
            upwardSpeed                     // Strong Z (upward) movement
            );

        p.speed = direction;

        // Colorful burst
        p.color[0] = 215;
        p.color[1] = 195;
        p.color[2] = 145;
        p.color[3] = 200 + rand() % 56;

        p.size = (rand() % 300) / 10000.0f + 0.01f; // Smaller particles (0.01-0.04)
    }
}
