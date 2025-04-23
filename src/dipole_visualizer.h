#pragma once

#include <vector>
#include <set>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "transform.h"
#include "shader.h"
#include "dipole.h"

class DipoleVisualizer : public Transform {
public:
    DipoleVisualizer(float radius = 0.1f, float arrow_length = 0.2f, MagneticDipole* parent = nullptr,
        const glm::vec4& northFaceColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        const glm::vec4& southFaceColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
        const glm::vec4& edgeColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    ~DipoleVisualizer();

    void initialize();
    void render(const glm::mat4& view, const glm::mat4& projection, Shader& shader);
    void printVAO();

private:
    float m_radius;
    float m_arrow_length;
    glm::vec4 m_northFaceColor;
    glm::vec4 m_southFaceColor;
    glm::vec4 m_edgeColor;
    unsigned int sphere_VAO{ 0 }, sphere_VBO{ 0 }, sphere_EBO{ 0 };
    unsigned int arrow_VAO{ 0 }, arrow_VBO{ 0 }, arrow_EBO{ 0 };
    std::vector<float> sphere_vertices;
    std::vector<float> sphere_colors; // Added for per-vertex colors
    std::vector<unsigned int> sphere_indices;
    std::vector<float> arrow_vertices;
    std::vector<unsigned int> arrow_indices;

    static std::set<unsigned int> active_VAOs; // Track active VAO IDs

    void generateSphereGeometry(int sectors = 16, int stacks = 16);
    void generateArrowGeometry();
    void setupBuffers();
    bool isVAOValid(unsigned int vao) const;
    void checkGLError(const std::string& context);
};