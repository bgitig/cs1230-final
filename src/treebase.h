#pragma once
#include "turtle.h"
#include <string>
#include <map>

class TreeBase {
public:
    TreeBase();

    // Generate L-system string through iterations
    void generateString(std::string sentence, int depth);

    // Interpret the L-system string and generate 3D geometry
    void drawPattern(std::string sentence, float turnAngle);

    // Get the generated L-system string
    std::string getGeneratedString() const { return m_generatedString; }

    // Get vertex data from turtle
    const std::vector<float>& getVertexData() const { return m_turtle.getVertexData(); }
    int getVertexCount() const { return m_turtle.getVertexCount(); }

    // Tree generation with presets
    void generateTree(const std::string& preset, int iterations);

    // Get available preset names
    std::vector<std::string> getPresetNames() const;

private:
    std::string m_generatedString;
    turtle m_turtle;

    struct TreePreset {
        std::string axiom;
        std::map<char, std::string> rules;
        float angle;
        std::string description;
    };

    std::map<std::string, TreePreset> m_presets;
    void initializePresets();
};
