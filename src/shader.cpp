#include "shader.h"

Shader::Shader(const char* vertex_shader_source, const char* fragment_shader_source) {
    // Create shader program
    program_ID = glCreateProgram();

    // Attach vertex and fragment shaders
    add_shader(program_ID, vertex_shader_source, GL_VERTEX_SHADER);
    add_shader(program_ID, fragment_shader_source, GL_FRAGMENT_SHADER);

    // Link the program
    glLinkProgram(program_ID);

    // Check for linking errors
    int success;
    char error_message[512];
    glGetProgramiv(program_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program_ID, 512, nullptr, error_message);
        std::cout << "Error linking shader program: " << error_message << "\n";
    }
}

Shader::~Shader() {
    if (program_ID != 0) {
        glDeleteProgram(program_ID);
        program_ID = 0;
    }
}

void Shader::use_shader() {
    glUseProgram(program_ID);
}

void Shader::add_shader(unsigned int program, const char* shader_source, GLenum shader_type) {
    // Create shader object
    unsigned int shader = glCreateShader(shader_type);

    // Set shader source and compile
    const GLchar* code[1] = { shader_source };
    glShaderSource(shader, 1, code, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    char error_message[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, error_message);
        std::cout << "Error compiling shader: " << error_message << "\n";
    }

    // Attach shader to program and delete shader object
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void Shader::set_float(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(program_ID, name.c_str()), value);
}

void Shader::set_vec2(const std::string& name, glm::vec2 vec) const {
    glUniform2f(glGetUniformLocation(program_ID, name.c_str()), vec.x, vec.y);
}

void Shader::set_vec3(const std::string& name, glm::vec3 vec) const {
    glUniform3f(glGetUniformLocation(program_ID, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::set_vec4(const std::string& name, glm::vec4 vec) const {
    glUniform4f(glGetUniformLocation(program_ID, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void Shader::set_mat4(const std::string& name, glm::mat4 mat) const {
    glUniformMatrix4fv(glGetUniformLocation(program_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::set_int(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(program_ID, name.c_str()), value);
}