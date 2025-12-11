#include "objloader.h"
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <QFile>
#include <QTextStream>
#include <QDebug>

bool OBJLoader::loadOBJ(const std::string& path, std::vector<float>& out_vertices) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> vertexIndices, normalIndices;

    QString qpath = QString::fromStdString(path);
    QFile file(qpath);

    if (!file.exists()) {
        std::cerr << "ERROR: File does not exist: " << path << std::endl;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        std::cerr << "Error: " << file.errorString().toStdString() << std::endl;
        return false;
    }

    QTextStream in(&file);
    QString line;
    int lineNum = 0;

    while (!in.atEnd()) {
        lineNum++;
        line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) {
            continue; // skip empty lines and comments
        }

        std::string stdLine = line.toStdString();
        std::istringstream iss(stdLine);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 vertex;
            if (!(iss >> vertex.x >> vertex.y >> vertex.z)) {
                std::cerr << "WARNING: Invalid vertex data at line " << lineNum << std::endl;
                continue;
            }
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            if (!(iss >> normal.x >> normal.y >> normal.z)) {
                std::cerr << "WARNING: Invalid normal data at line " << lineNum << std::endl;
                continue;
            }
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            std::vector<std::string> faceTokens;
            std::string token;

            // collect all faces
            while (iss >> token) {
                faceTokens.push_back(token);
            }

            // polygone triangulation
            for (size_t i = 1; i + 1 < faceTokens.size(); i++) {
                // vertices 0, i, i+1
                for (size_t j = 0; j < 3; j++) {
                    size_t idx = (j == 0) ? 0 : (j == 1 ? i : i + 1);
                    std::string& faceData = faceTokens[idx];

                    std::istringstream viss(faceData);
                    std::string vIdx, vtIdx, vnIdx;

                    std::getline(viss, vIdx, '/');
                    std::getline(viss, vtIdx, '/');
                    std::getline(viss, vnIdx, '/');

                    if (!vIdx.empty()) {
                        vertexIndices.push_back(std::stoi(vIdx));
                    }
                    if (!vnIdx.empty()) {
                        normalIndices.push_back(std::stoi(vnIdx));
                    }
                }
            }
        }
    }

    file.close();

    out_vertices.clear();
    for (size_t i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i] - 1;

        if (vertexIndex >= temp_vertices.size()) {
            std::cerr << "ERROR: Vertex index out of bounds." << std::endl;
            return false;
        }

        glm::vec3 vertex = temp_vertices[vertexIndex];
        out_vertices.push_back(vertex.x);
        out_vertices.push_back(vertex.y);
        out_vertices.push_back(vertex.z);

        if (i < normalIndices.size()) {
            unsigned int normalIndex = normalIndices[i] - 1;

            if (normalIndex < temp_normals.size()) {
                glm::vec3 normal = temp_normals[normalIndex];
                out_vertices.push_back(normal.x);
                out_vertices.push_back(normal.y);
                out_vertices.push_back(normal.z);
            } else {
                out_vertices.push_back(0.0f);
                out_vertices.push_back(0.0f);
                out_vertices.push_back(1.0f);
            }
        } else {
            out_vertices.push_back(0.0f);
            out_vertices.push_back(0.0f);
            out_vertices.push_back(1.0f);
        }
    }

    std::cout << "Successfully loaded OBJ" << std::endl;
    return true;
}
