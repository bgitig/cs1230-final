#include "skybox.h"
#include <QImage>
#include <QDebug>

// Skybox vertices
float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

skybox::skybox() : textureID(0), VAO(0), VBO(0), shaderProgram(0) {}

void skybox::init() {
    // Load cubemap textures - all faces use the same image for now
    std::vector<std::string> faces = {
        ":/images/right.jpg",    // GL_TEXTURE_CUBE_MAP_POSITIVE_X (right)
        ":/images/left.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X (left)
        ":/images/front.jpg",    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y (top)
        ":/images/back.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (bottom)
        ":/images/top.jpg",    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z (front)
        ":/images/bottom.jpg"     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (back)
    };

    textureID = loadCubemap(faces);
    setupSkyboxVertices();
    compileShaders();
}

unsigned char* skybox::loadImage(const char* filename, int* width, int* height, int* nrChannels) {
    QImage image;
    if (!image.load(filename)) {
        qDebug() << "Failed to load image:" << filename;
        return nullptr;
    }

    // CRITICAL FIX: Ensure power-of-2 dimensions for better compatibility
    // Also ensure all faces have the same dimensions
    int targetSize = 512; // Use a reasonable power-of-2 size
    if (image.width() != targetSize || image.height() != targetSize) {
        image = image.scaled(targetSize, targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
       // qDebug() << "Resized image to:" << targetSize << "x" << targetSize;
    }

    // Convert to RGB format (not RGBA) for better compatibility
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored(); // Flip vertically for OpenGL

    *width = image.width();
    *height = image.height();
    *nrChannels = 3; // RGB

    // Copy image data
    size_t dataSize = image.sizeInBytes();
    unsigned char* data = new unsigned char[dataSize];
    memcpy(data, image.constBits(), dataSize);

    return data;
}

unsigned int skybox::loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    if (textureID == 0) {
        qDebug() << "ERROR: Failed to generate texture ID";
        return 0;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    int width, height, nrChannels;
    bool allLoaded = true;
    int firstWidth = -1, firstHeight = -1;

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = loadImage(faces[i].c_str(), &width, &height, &nrChannels);

        if (data) {

            if (firstWidth == -1) {
                firstWidth = width;
                firstHeight = height;
            } else if (width != firstWidth || height != firstHeight) {
                qDebug() << "ERROR: Face" << i << "has different dimensions!"
                         << width << "x" << height << "vs" << firstWidth << "x" << firstHeight;
                delete[] data;
                allLoaded = false;
                continue;
            }

            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;

            // Load the texture
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                qDebug() << "OpenGL error after loading face" << i << ":" << error;
                if (error == GL_INVALID_VALUE) {
                    qDebug() << "GL_INVALID_VALUE - Check texture dimensions:" << width << "x" << height;
                }
                allLoaded = false;
            }

            delete[] data;
        } else {
            qDebug() << "Failed to load cubemap face" << i << ":" << faces[i].c_str();
            allLoaded = false;
         }
    }

    if (!allLoaded) {
        qDebug() << "WARNING: Using fallback colors for some skybox faces.";
    }

    // Unbind the cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Final error check
    GLenum finalError = glGetError();
    if (finalError != GL_NO_ERROR) {
        qDebug() << "Final OpenGL error after cubemap setup:" << finalError;
    }

    return textureID;
}

void skybox::setupSkyboxVertices() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void skybox::compileShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        out vec3 TexCoords;

        uniform mat4 projection;
        uniform mat4 view;

        void main() {
            TexCoords = aPos;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 TexCoords;
        out vec4 FragColor;

        uniform samplerCube skybox;

        void main() {
            FragColor = texture(skybox, TexCoords);
        }
    )";

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "Vertex shader compilation failed:" << infoLog;
    }
    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "Fragment shader compilation failed:" << infoLog;
    }

    // Link shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        qDebug() << "Shader program linking failed:" << infoLog;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void skybox::render(const float* view, const float* projection) {
    // Early return if texture isn't valid
    if (textureID == 0 || VAO == 0 || shaderProgram == 0) {
        qDebug() << "Skybox not properly initialized, skipping render";
        return;
    }

    glDepthFunc(GL_LEQUAL);

    glUseProgram(shaderProgram);

    // Remove translation from view matrix
    float viewNoTranslation[16];
    for(int i = 0; i < 16; i++) viewNoTranslation[i] = view[i];
    viewNoTranslation[12] = viewNoTranslation[13] = viewNoTranslation[14] = 0.0f;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, viewNoTranslation);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);

    // Bind skybox texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);

    // Draw skybox
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glUseProgram(0);
}
