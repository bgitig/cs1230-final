#include "bumpmapping.h"
#include "utils/shaderloader.h"
#include <iostream>

BumpMapping::BumpMapping()
    : m_enabled(false),
    m_bumpStrength(1.5f),
    m_bumpScale(8.0f),
    m_bumpVariation(0.8f),
    m_bumpSeed(0.0f),
    m_lightDir(0.5f, 1.0f, 0.5f),
    m_bumpShader(0),
    m_initialized(false) {
}

BumpMapping::~BumpMapping() {
    cleanup();
}

void BumpMapping::initialize() {
    if (m_initialized) return;

    m_bumpShader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/bump.vert",
        ":/resources/shaders/bump.frag"
        );

    m_initialized = true;
    std::cout << "Bump mapping initialized (Render-Shift-Subtract)" << std::endl;
}

void BumpMapping::cleanup() {
    if (m_bumpShader != 0) {
        glDeleteProgram(m_bumpShader);
        m_bumpShader = 0;
    }
    m_initialized = false;
}
