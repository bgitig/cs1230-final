#include "treemanager.h"
#include <iostream>

TreeManager::TreeManager()
    : m_vao(0), m_vbo(0), m_vertexCount(0), m_hasTree(false), m_glInitialized(false) {
}

TreeManager::~TreeManager() {
    cleanup();
}

void TreeManager::initialize() {
    if (m_glInitialized) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    m_glInitialized = true;
}

void TreeManager::generateTree(const std::string& preset, int iterations) {
    if (!m_glInitialized) {
        return;
    }

    m_treeGenerator.generateTree(preset, iterations);

    uploadToGPU();

    m_hasTree = true;
}

void TreeManager::generateCustomTree(const std::string& axiom,
                                     const std::string& rules,
                                     int iterations,
                                     float angle) {
    if (!m_glInitialized) {
        return;
    }

    m_treeGenerator.generateString(axiom, iterations);
    std::string generated = m_treeGenerator.getGeneratedString();

    m_treeGenerator.drawPattern(generated, angle);

    uploadToGPU();

    m_hasTree = true;
}

void TreeManager::uploadToGPU() {
    const std::vector<float>& vertexData = m_treeGenerator.getVertexData();
    m_vertexCount = m_treeGenerator.getVertexCount();

    if (vertexData.empty()) {
        return;
    }
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TreeManager::draw() {
    if (!m_hasTree || m_vertexCount == 0) return;

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}

void TreeManager::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    m_glInitialized = false;
}
