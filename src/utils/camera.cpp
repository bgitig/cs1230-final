#include <stdexcept>
#include "camera.h"
#include "utils/scenedata.h"
#include <iostream>

void Camera::cameraSetUp(SceneCameraData cameraData, int w, int h) {
    cData = cameraData;
    viewMatrix = calculateViewMatrix(glm::vec3(cData.look), glm::vec3(cData.up), cData.pos);
    width = w;
    height = h;
}

glm::mat4 Camera::calculateProjectionMatrix(float near, float far) {
    glm::mat4 opengl(1.0f,0,0,0,
                     0,1.0f,0,0,
                    0,0,-2.0f,0,
                     0,0,-1.0f,1.0f);

    float c = -near/far;
    glm::mat4 Mpp(1.0f,0,0,0,
                    0,1.0f,0,0,
                  0,0,1.0f/(1.0f+c),-1.0f,
                  0,0,-c/(1.0f+c),0);

    glm::mat4 scale(1.0f/(far*tan(getWidthAngle()/2.0f)),0,0,0,
                    0,1.0f/(far*tan(getHeightAngle()/2.0f)),0,0,
                    0,0,1.0f/far,0,
                    0,0,0,1.0f);

    return opengl*Mpp*scale;

}

glm::mat4 Camera::calculateViewMatrix(glm::vec3 look, glm::vec3 up, glm::vec4 pos) {
    // glm::vec3 look = glm::vec3(cData.look);
    // glm::vec3 up = glm::vec3(cData.up);

    glm::vec3 w = glm::normalize(-look);
    glm::vec3 v = glm::normalize(up-glm::dot(up,w)*w);
    glm::vec3 u = glm::cross(v, w);

    glm::mat4 R(glm::vec4(u[0],v[0],w[0],0),
                glm::vec4(u[1],v[1],w[1],0),
                glm::vec4(u[2],v[2],w[2],0),
                glm::vec4(0,0,0,1));

    // glm::vec4 pos = cData.pos;
    glm::mat4 T(glm::vec4(1,0,0,0),
                glm::vec4(0,1,0,0),
                glm::vec4(0,0,1,0),
                glm::vec4(-pos[0],-pos[1],-pos[2],1));
    return R*T;
}

glm::mat4 Camera::getViewMatrix() const {
    // Optional TODO: implement the getter or make your own design
    return viewMatrix;
}

float Camera::getAspectRatio() const {
    // Optional TODO: implement the getter or make your own design
    return (float)width/(float)height;
}

float Camera::getHeightAngle() const {
    // Optional TODO: implement the getter or make your own design
    return cData.heightAngle;
}

float Camera::getWidthAngle() const {
    return 2*atan(getAspectRatio()*tan(getHeightAngle()/2));
}

float Camera::getFocalLength() const {
    // Optional TODO: implement the getter or make your own design
    return cData.focalLength;
}

float Camera::getAperture() const {
    // Optional TODO: implement the getter or make your own design
    return cData.aperture;
}
