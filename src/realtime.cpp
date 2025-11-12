#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "glm/gtc/matrix_transform.hpp"

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

    // If you must use this function, do not edit anything above this
}

Mesh Realtime::createMesh(const std::vector<GLfloat> &shapeData) {
    Mesh mesh;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, shapeData.size() * sizeof(GLfloat), shapeData.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat)*3));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh.vertexCount = shapeData.size() / 6;
    return mesh;
}

Shape* makeShape(PrimitiveType type, int param1, int param2) {
    switch (type) {
    // case PrimitiveType::PRIMITIVE_CONE:
    //     return new Cone(param1, param2);
    // case PrimitiveType::PRIMITIVE_CUBE:
    //     return new Cube(param1, param2);
    // case PrimitiveType::PRIMITIVE_CYLINDER:
    //     return new Cylinder(param1, param2);
    case PrimitiveType::PRIMITIVE_SPHERE:
        std::cout<<"making sphere"<<std::endl;
        return new Sphere(param1, param2);
        break;
    default:
        return nullptr;
    }
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    for (Mesh mesh : m_meshes) {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
    glDeleteProgram(m_shader);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    glClearColor(0,0,0,1);
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");

}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader);

    for (Mesh &mesh : m_meshes) {
        glBindVertexArray(vao);

        m_mvp = m_proj * m_view * mesh.shape.ctm;
        GLint mvpLoc = glGetUniformLocation(m_shader, "m_mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &m_mvp[0][0]);

        m_model = mesh.shape.ctm;
        GLint model_location = glGetUniformLocation(m_shader, "m_model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, &m_model[0][0]);

        // GLint ictmLoc = glGetUniformLocation(m_shader, "ictm");
        // glUniformMatrix3fv(ictmLoc, 1, GL_FALSE, &mesh.shape.ictm[0][0]);
        // GLint view_location = glGetUniformLocation(m_shader, "m_view");
        // glUniformMatrix4fv(view_location, 1, GL_FALSE, &m_view[0][0]);
        // GLint proj_location = glGetUniformLocation(m_shader, "m_proj");
        // glUniformMatrix4fv(proj_location, 1, GL_FALSE, &m_proj[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        glBindVertexArray(0);
    }
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::sceneChanged() {
    bool success = SceneParser::parse(settings.sceneFilePath, renderData);
    if (!success) return;

    for (Mesh &mesh : m_meshes) {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
    m_meshes.clear();

    for (RenderShapeData shape : renderData.shapes) {
        Shape* shapeData = makeShape(shape.primitive.type, settings.shapeParameter1, settings.shapeParameter2);
        std::vector<float> shapeVertexData = shapeData->getVertexData();
        for (int i = 0; i < 10; i++) {
            std::cout<<"shapedata at "<<i<<"is: "<<shapeVertexData[i]<<std::endl;
        }
        Mesh mesh = createMesh(shapeVertexData);
        mesh.shape = shape;
        m_meshes.push_back(mesh);
    }

    // for (SceneLightData light : renderData.lights) {
    // for (int i = 0; i < renderData.lights.size(); i++) {
    //     GLint lightPosLoc = glGetUniformLocation(m_shader, "m_lightPosArray["+std::to_string(i)+"]");
    //     glm::vec4 lightPos = renderData.lights[i].pos;
    //     glUniform4f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z, lightPos.w);

    //     GLint lightDirLoc = glGetUniformLocation(m_shader, "m_lightDirArray["+std::to_string(i)+"]");
    //     glm::vec4 lightDir = renderData.lights[i].dir;
    //     gluniform4f(lightDirLoc, lightDir.x, lightDir.y, lightDir.z, lightDir.w);

    //     renderData.lights[i];
    // }

    SceneCameraData camData = renderData.cameraData;

    camera.cameraSetUp(camData, size().width(), size().height());
    camLook = glm::vec3(camData.look);
    camUp = glm::vec3(camData.up);
    camPos = camData.pos;

    m_view = camera.calculateViewMatrix(camLook, camUp, camPos);
    m_proj = camera.calculateProjectionMatrix(settings.nearPlane, settings.farPlane);

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    // for (Mesh &mesh : m_meshes) {
    //     std::vector<float> newData = makeShape(mesh.shape.primitive.type, settings.shapeParameter1, settings.shapeParameter2)->getVertexData();
    //     glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    //     glBufferData(GL_ARRAY_BUFFER, newData.size() * sizeof(GLfloat), newData.data(), GL_STATIC_DRAW);
    //     mesh.vertexCount = newData.size() / 6;
    // }
    m_view = camera.calculateViewMatrix(camLook, camUp, camPos);
    m_proj = camera.calculateProjectionMatrix(settings.nearPlane, settings.farPlane);

    update(); // asks for a PaintGL() call to occur
}

// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
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
    // if (m_mouseDown) {
    //     int posX = event->position().x();
    //     int posY = event->position().y();
    //     int deltaX = posX - m_prev_mouse_pos.x;
    //     int deltaY = posY - m_prev_mouse_pos.y;
    //     m_prev_mouse_pos = glm::vec2(posX, posY);

    //     // Use deltaX and deltaY here to rotate
    //     float xAngle = deltaX * .005f;
    //     float yAngle = deltaY * .005f;

    //     // mouse x: rotate about (0,1,0)
    //     glm::vec3 xAxis(0.0f, 1.0f, 0.0f);
    //     glm::mat3 xRot = rotMat(xAxis, xAngle);
    //     camLook = glm::normalize(xRot*camLook);
    //     camUp = glm::normalize(camUp*xRot);

    //     // mouse y: rotate about axis defined by vector perpindicular to look and up
    //     glm::vec3 yAxis = glm::normalize(glm::cross(camLook, camUp));
    //     glm::mat3 yRot = rotMat(yAxis, yAngle);
    //     camLook = glm::normalize(yRot*camLook);
    //     camUp = glm::normalize(yRot*camUp);

        // // Optionally clamp vertical rotation to prevent flipping
        // if (glm::abs(glm::dot(newFront, worldUp)) < 0.99f) {
        //     m_cameraFront = newFront;
        //     m_cameraUp = newUp;
        // }
\
        update(); // asks for a PaintGL() call to occur
    // }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    float velocity = 5.0f * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(camLook, camUp));

    // if (m_keyMap[Qt::Key_W]) camPos += glm::vec4(camLook * velocity, 0.0f);
    // if (m_keyMap[Qt::Key_A]) camPos -= glm::vec4(right * velocity, 0.0f);
    // if (m_keyMap[Qt::Key_S]) camPos -= glm::vec4(camLook * velocity, 0.0f);
    // if (m_keyMap[Qt::Key_D]) camPos += glm::vec4(right * velocity, 0.0f);

    // // // change to be along (0,1 or -1,0)
    // if (m_keyMap[Qt::Key_Control]) camPos -= glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * velocity;
    // if (m_keyMap[Qt::Key_Space]) camPos += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * velocity;

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
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

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
