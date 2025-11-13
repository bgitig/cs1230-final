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

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);
    glDeleteBuffers(1, &m_cone_vbo);
    glDeleteVertexArrays(1, &m_cone_vao);
    glDeleteBuffers(1, &m_cube_vbo);
    glDeleteVertexArrays(1, &m_cube_vao);
    glDeleteBuffers(1, &m_cylinder_vbo);
    glDeleteVertexArrays(1, &m_cylinder_vao);

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
    setUp();
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
    // Students: anything requiring OpenGL calls every frame should be done here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader);

    for (RenderShapeData shape : renderData.shapes) {
        glBindVertexArray(typeInterpretVao(shape.primitive.type));

        m_mvp = m_proj * m_view * shape.ctm;
        GLint mvpLoc = glGetUniformLocation(m_shader, "m_mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &m_mvp[0][0]);

        m_model = shape.ctm;
        GLint model_location = glGetUniformLocation(m_shader, "m_model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, &m_model[0][0]);

        ictm = shape.ictm;
        GLint ictmLoc = glGetUniformLocation(m_shader, "ictm");
        glUniformMatrix3fv(ictmLoc, 1, GL_FALSE, &ictm[0][0]);

        GLint cameraPosLoc = glGetUniformLocation(m_shader, "camera_pos");
        glUniform4fv(cameraPosLoc, 1, &camPos[0]);

        GLint ka_location = glGetUniformLocation(m_shader, "m_ka");
        glUniform1f(ka_location, m_ka);
        GLint kd_location = glGetUniformLocation(m_shader, "m_kd");
        glUniform1f(kd_location, m_kd);
        GLint ks_location = glGetUniformLocation(m_shader, "m_ks");
        glUniform1f(ks_location, m_ks);

        // m_lightPos = inverse(m_model) * m_lightPos;
        // GLint light_location = glGetUniformLocation(m_shader, "m_lightPos");
        // glUniform4fv(light_location, 1, &m_lightPos[0]);


        SceneMaterial material = shape.primitive.material;
        GLint cAmbient_location = glGetUniformLocation(m_shader, "m_cAmbient");
        glUniform4f(cAmbient_location, material.cAmbient.x, material.cAmbient.y, material.cAmbient.z, material.cAmbient.w);
        GLint cDiffuse_location = glGetUniformLocation(m_shader, "m_cDiffuse");
        glUniform4f(cDiffuse_location, material.cDiffuse.x, material.cDiffuse.y, material.cDiffuse.z, material.cDiffuse.w);
        GLint cSpecular_location = glGetUniformLocation(m_shader, "m_cSpecular");
        glUniform4f(cSpecular_location, material.cSpecular.x, material.cSpecular.y, material.cSpecular.z, material.cSpecular.w);
        GLint shininess_location = glGetUniformLocation(m_shader, "m_shininess");
        glUniform1f(shininess_location, material.shininess);
        GLint cReflectivelocation = glGetUniformLocation(m_shader, "m_cReflective");
        glUniform4f(cReflectivelocation, material.cReflective.x, material.cReflective.y, material.cReflective.z, material.cReflective.w);


        glDrawArrays(GL_TRIANGLES, 0, typeInterpretVertices(shape.primitive.type));
        glBindVertexArray(0);
    }

    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::updateCamera() {
    m_view = camera.calculateViewMatrix(camLook, camUp, camPos);
    m_proj = camera.calculateProjectionMatrix(settings.nearPlane, settings.farPlane);
    // camPos = glm::inverse(m_view)*glm::vec4(0,0,0,1);
}

void Realtime::updateLights() {
    glUseProgram(m_shader);
    std::vector<SceneLightData> lights = renderData.lights;
    for (int i = 0; i < lights.size(); i++) {
        std::cout<<"updating lights"<<std::endl;
        GLint wloc = glGetUniformLocation(m_shader, "white");
        glUniform1f(wloc, 1.0f);

        SceneLightData light = lights[i];

        LightType type = light.type;
        int typeInt = type == LightType::LIGHT_POINT ? 0 : (type == LightType::LIGHT_DIRECTIONAL ? 1 : 2);
        GLint lightTypeLoc = glGetUniformLocation(m_shader, ("m_lightType["+std::to_string(i)+"]").c_str());
        glUniform1i(lightTypeLoc, typeInt);

        GLint lightColorLoc = glGetUniformLocation(m_shader, ("m_lightColor["+std::to_string(i)+"]").c_str());
        glm::vec4 lightColor = light.color;
        glUniform4f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z, lightColor.w);

        GLint lightFunctionLoc = glGetUniformLocation(m_shader, ("m_lightFunction["+std::to_string(i)+"]").c_str());
        glm::vec3 lightFunction= light.function;
        glUniform3f(lightFunctionLoc, lightFunction.x, lightFunction.y, lightFunction.z);

        // might need to multiply light pos by inverse ctm
        GLint lightPosLoc = glGetUniformLocation(m_shader, ("m_lightPos["+std::to_string(i)+"]").c_str());
        glm::vec4 lightPos = light.pos;
        glUniform4f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z, lightPos.w);

        GLint lightDirLoc = glGetUniformLocation(m_shader, ("m_lightDir["+std::to_string(i)+"]").c_str());
        glm::vec4 lightDir = light.dir;
        glUniform4f(lightDirLoc, lightDir.x, lightDir.y, lightDir.z, lightDir.w);

        GLint lightPenumbraLoc = glGetUniformLocation(m_shader, ("m_lightPenumbra["+std::to_string(i)+"]").c_str());
        glUniform1f(lightPenumbraLoc, light.penumbra);

        GLint lightAngleLoc = glGetUniformLocation(m_shader, ("m_lightAngle["+std::to_string(i)+"]").c_str());
        glUniform1f(lightAngleLoc, light.angle);

        // glm::vec3 directionToLight = light.type == LightType::LIGHT_DIRECTIONAL ? glm::vec3(glm::normalize(-light.dir)) : glm::normalize(glm::vec3(light.pos)-position);
        // GLint lightAngleLoc = glGetUniformLocation(m_shader, "m_lightAngle["+std::to_string(i)+"]");
        // glUniform1f(lightAngleLoc, lights[i].angle);
    }
    glUseProgram(0);
}

void Realtime::sceneChanged() {
    bool success = SceneParser::parse(settings.sceneFilePath, renderData);
    if (!success) return;

    SceneCameraData camData = renderData.cameraData;
    camera.cameraSetUp(camData, size().width(), size().height());
    camLook = glm::vec3(camData.look);
    camUp = glm::vec3(camData.up);
    camPos = camData.pos;
    updateCamera();
    updateLights();

    // for (SceneLightData light : renderData.lights) {


    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    if (isSetUp) updateShapes();

    // for (Mesh &mesh : m_meshes) {
    //     std::vector<float> newData = makeShape(mesh.shape.primitive.type, settings.shapeParameter1, settings.shapeParameter2)->getVertexData();
    //     glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    //     glBufferData(GL_ARRAY_BUFFER, newData.size() * sizeof(GLfloat), newData.data(), GL_STATIC_DRAW);
    //     mesh.vertexCount = newData.size() / 6;
    // }
    std::cout<<"settings changed"<<std::endl;
    // m_view = camera.calculateViewMatrix(camLook, camUp, camPos);
    // m_proj = camera.calculateProjectionMatrix(settings.nearPlane, settings.farPlane);

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
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        float xAngle = deltaX * .005f;
        float yAngle = deltaY * .005f;

        // mouse x: rotate about (0,1,0)
        glm::vec3 xAxis(0.0f, 1.0f, 0.0f);
        glm::mat3 xRot = rotMat(xAxis, xAngle);
        camLook = glm::normalize(xRot*camLook);
        camUp = glm::normalize(camUp*xRot);

        // mouse y: rotate about axis defined by vector perpindicular to look and up
        glm::vec3 yAxis = glm::normalize(glm::cross(camLook, camUp));
        glm::mat3 yRot = rotMat(yAxis, yAngle);
        camLook = glm::normalize(yRot*camLook);
        camUp = glm::normalize(yRot*camUp);

        // // Optionally clamp vertical rotation to prevent flipping
        // if (glm::abs(glm::dot(newFront, worldUp)) < 0.99f) {
        //     m_cameraFront = newFront;
        //     m_cameraUp = newUp;
        // }
        updateCamera();
\
        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    float velocity = 5.0f * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(camLook, camUp));

    if (m_keyMap[Qt::Key_W]) camPos -= glm::vec4(camLook * velocity, 0.0f);
    if (m_keyMap[Qt::Key_A]) camPos += glm::vec4(right * velocity, 0.0f);
    if (m_keyMap[Qt::Key_S]) camPos += glm::vec4(camLook * velocity, 0.0f);
    if (m_keyMap[Qt::Key_D]) camPos -= glm::vec4(right * velocity, 0.0f);

    // // // change to be along (0,1 or -1,0)
    if (m_keyMap[Qt::Key_Control]) camPos += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * velocity;
    if (m_keyMap[Qt::Key_Space]) camPos -= glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * velocity;
    updateCamera();

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
