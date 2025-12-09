#include "treebase.h"
#include <cmath>
#include <iostream>

TreeBase::TreeBase() {
    initializePresets();
}

void TreeBase::initializePresets() {
    m_presets["Original"] = {
                             "F",
                             {
                                 {'F', "F[+&F]F[-^F]F"}
                             },
                             22.5f,
        "Complex 3D tree with multiple branches"
    };

    m_presets["Simple"] = {
                           "F",
                           {
                               {'F', "F[+&F]F[-F]F[<^F]"}
                           },
                           22.5f,
                           "Very natural-looking asymmetric 3D tree"
    };

    m_presets["Bush"] = {
        "X",
        {
            { 'X', "F[+X][-X]FX" },
            { 'F', "FF" }
        },
        22.0f,
    };

    m_presets["Asymmetric"] = {
                               "F",
                               {
                                   {'F', "F[+&F]F[-^F]F"}
                               },
                               22.5f,
        "Asymmetric branching tree"
    };

    m_presets["Dense"] = {
                          "F",
                          {
                              { 'F', "FF-[-F+F+F]+[+F-F-F]" }
                          },
                          20.0f,
        "Dense tree with many branches"
    };
}

std::vector<std::string> TreeBase::getPresetNames() const {
    std::vector<std::string> names;
    for (const auto& pair : m_presets) {
        names.push_back(pair.first);
    }
    return names;
}

void TreeBase::drawPattern(std::string sentence, float turnAngle) {
    m_turtle.reset();
    int draw_num = 0;
    bool trunk = true;

    for (size_t i = 0; i < sentence.length(); i++) {
        char current = sentence[i];

        if (current == 'F' || current == 'G') {
            m_turtle.draw();
            if (draw_num++ % 5 == 0 && trunk) {
                m_turtle.setThickness(-1);
            }
        } else if (current == 'f') {
            m_turtle.moveForward(0.2f);
        } else if (current == '+') {
            m_turtle.rotate(-turnAngle);
        } else if (current == '-') {
            m_turtle.rotate(turnAngle);
        } else if (current == '&') {
            m_turtle.pitchDown(turnAngle);
        } else if (current == '^') {
            m_turtle.pitchUp(turnAngle);
        } else if (current == '<') {
            m_turtle.rollLeft(turnAngle);
        } else if (current == '>') {
            m_turtle.rollRight(turnAngle);
        } else if (current == '|') {
            m_turtle.turnAround();
        } else if (current == '[') {
            m_turtle.saveState();
            m_turtle.setThickness(0);
            trunk = false;
        } else if (current == ']') {
            m_turtle.restoreState();
            m_turtle.setThickness(0);
        } else if (current == 'X' && i + 1 < sentence.length() && sentence[i + 1] != '[') {
            trunk = false;
        }
    }

}

void TreeBase::generateString(std::string sentence, int depth) {
    if (depth == 0) {
        m_generatedString = sentence;
        return;
    }

    std::string nextSentence;
    for (size_t i = 0; i < sentence.length(); i++) {
        char current = sentence[i];

        if (current == 'X') {
            nextSentence += "F[- & < F][ < + + & F ] | | F [ - - & > F ][+ & F ]";
        } else if (current == 'F') {
            nextSentence += "FF";
        } else {
            nextSentence += current;
        }
    }

    generateString(nextSentence, depth - 1);
}

void TreeBase::generateTree(const std::string& preset, int iterations) {
    auto it = m_presets.find(preset);
    if (it == m_presets.end()) {
        it = m_presets.find("Original");
    }

    TreePreset& p = it->second;

    std::string current = p.axiom;
    for (int i = 0; i < iterations; i++) {
        std::string next = "";
        for (char c : current) {
            auto ruleIt = p.rules.find(c);
            if (ruleIt != p.rules.end()) {
                next += ruleIt->second;
            } else {
                next += c;
            }
        }
        current = next;
    }

    m_generatedString = current;
    std::cout << "Generated L-system string (length: " << current.length() << ")" << std::endl;

    drawPattern(current, p.angle);
}
