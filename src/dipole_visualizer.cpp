#include "dipole_visualizer.h"

std::set<unsigned int> DipoleVisualizer::active_VAOs = {};

DipoleVisualizer::DipoleVisualizer(float radius, float arrow_length, MagneticDipole* parent,
    const glm::vec4& northFaceColor, const glm::vec4& southFaceColor, const glm::vec4& edgeColor)
    : Transform(glm::vec3(0.0f), glm::vec3(0.0f), parent),
    m_radius(radius), m_arrow_length(arrow_length),
    m_northFaceColor(northFaceColor), m_southFaceColor(southFaceColor), m_edgeColor(edgeColor) {
}

void DipoleVisualizer::printVAO() {
    std::cout << "Dipole with sphere and arrow VAO: " << sphere_VAO << ", " << arrow_VAO << std::endl;
}

DipoleVisualizer::~DipoleVisualizer() {
    std::cout << "Removing dipole with sphere and arrow VAO: " << sphere_VAO << ", " << arrow_VAO << std::endl;
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (sphere_VAO) {
        glDeleteVertexArrays(1, &sphere_VAO);
        active_VAOs.erase(sphere_VAO);
    }
    if (sphere_VBO) glDeleteBuffers(1, &sphere_VBO);
    if (sphere_EBO) glDeleteBuffers(1, &sphere_EBO);
    if (arrow_VAO) {
        glDeleteVertexArrays(1, &arrow_VAO);
        active_VAOs.erase(arrow_VAO);
    }
    if (arrow_VBO) glDeleteBuffers(1, &arrow_VBO);
    if (arrow_EBO) glDeleteBuffers(1, &arrow_EBO);
    checkGLError("DipoleVisualizer destructor");
}

void DipoleVisualizer::initialize() {
    generateSphereGeometry();
    generateArrowGeometry();
    setupBuffers();
}

void DipoleVisualizer::generateSphereGeometry(int sectors, int stacks) {
    sphere_vertices.clear();
    sphere_colors.clear();
    sphere_indices.clear();

    float sectorStep = 2.0f * glm::pi<float>() / sectors;
    float stackStep = glm::pi<float>() / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2.0f - i * stackStep;
        float xy = m_radius * cos(stackAngle);
        float z = m_radius * sin(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);
            sphere_vertices.push_back(x);
            sphere_vertices.push_back(y);
            sphere_vertices.push_back(z);

            // Assign colors: red for z > 0 (north), blue for z <= 0 (south)
            glm::vec4 color = (z > 0.0f) ?
                glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : // Red (north)
                glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);  // Blue (south)
            sphere_colors.push_back(color.r);
            sphere_colors.push_back(color.g);
            sphere_colors.push_back(color.b);
            sphere_colors.push_back(color.a);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                sphere_indices.push_back(k1);
                sphere_indices.push_back(k2);
                sphere_indices.push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                sphere_indices.push_back(k1 + 1);
                sphere_indices.push_back(k2);
                sphere_indices.push_back(k2 + 1);
            }
        }
    }
}

void DipoleVisualizer::generateArrowGeometry() {
    arrow_vertices.clear();
    arrow_indices.clear();

    arrow_vertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, m_arrow_length
    };
    arrow_indices = { 0, 1 };
}

void DipoleVisualizer::setupBuffers() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Sphere setup
    glGenVertexArrays(1, &sphere_VAO);
    active_VAOs.insert(sphere_VAO);
    glGenBuffers(1, &sphere_VBO);
    glGenBuffers(1, &sphere_EBO);
    glBindVertexArray(sphere_VAO);

    // Vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere_vertices.size() * sizeof(float), sphere_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex colors
    unsigned int sphere_color_VBO;
    glGenBuffers(1, &sphere_color_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_color_VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere_colors.size() * sizeof(float), sphere_colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_indices.size() * sizeof(unsigned int), sphere_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Arrow setup
    glGenVertexArrays(1, &arrow_VAO);
    active_VAOs.insert(arrow_VAO);
    glGenBuffers(1, &arrow_VBO);
    glGenBuffers(1, &arrow_EBO);
    glBindVertexArray(arrow_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, arrow_VBO);
    glBufferData(GL_ARRAY_BUFFER, arrow_vertices.size() * sizeof(float), arrow_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrow_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arrow_indices.size() * sizeof(unsigned int), arrow_indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::cout << "New dipole with sphere and arrow VAO: " << sphere_VAO << ", " << arrow_VAO << std::endl;

    checkGLError("setupBuffers");
}

bool DipoleVisualizer::isVAOValid(unsigned int vao) const {
    if (vao == 0) {
        std::cerr << "VAO is 0 (not initialized)" << std::endl;
        return false;
    }
    if (active_VAOs.find(vao) == active_VAOs.end()) {
        std::cerr << "VAO " << vao << " not in active_VAOs set" << std::endl;
        return false;
    }
    GLint isValid;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &isValid);
    glBindVertexArray(vao);
    bool valid = glGetError() == GL_NO_ERROR;
    glBindVertexArray(isValid);
    return valid;
}

void DipoleVisualizer::render(const glm::mat4& view, const glm::mat4& projection, Shader& shader) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in render: " << err << std::endl;
    }
    // Verify shader program
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    shader.use_shader();
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    // Set uniforms
    glm::mat4 model = getWorldTransformMatrix();
    shader.set_mat4("view", view);
    shader.set_mat4("projection", projection);
    shader.set_mat4("model", model);

    // Render sphere with vertex colors
    if (isVAOValid(sphere_VAO)) {
        glBindVertexArray(sphere_VAO);
        checkGLError("After binding sphere_VAO");
        if (sphere_indices.size() > 0) {
            glDrawElements(GL_TRIANGLES, sphere_indices.size(), GL_UNSIGNED_INT, 0);
            checkGLError("After drawing sphere");
        }
        else {
            std::cerr << "sphere_indices empty, skipping draw" << std::endl;
        }
        glBindVertexArray(0);
    }

    // Render arrow with uniform color
    shader.set_vec4("color", m_edgeColor);
    if (isVAOValid(arrow_VAO)) {
        glBindVertexArray(arrow_VAO);
        checkGLError("After binding arrow_VAO");
        if (arrow_indices.size() > 0) {
            glDrawElements(GL_LINES, arrow_indices.size(), GL_UNSIGNED_INT, 0);
            checkGLError("After drawing arrow");
        }
        else {
            std::cerr << "arrow_indices empty, skipping draw" << std::endl;
        }
        glBindVertexArray(0);
    }
}

void DipoleVisualizer::checkGLError(const std::string& context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in " << context << ": " << err << std::endl;
    }
}