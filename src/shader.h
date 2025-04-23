#pragma once
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    unsigned int program_ID;
    // Updated constructor to take shader source strings
    Shader(const char* vertex_shader_source, const char* fragment_shader_source);
    ~Shader();

    void use_shader();

    void set_float(const std::string& name, float value) const;
    void set_vec2(const std::string& name, glm::vec2 vec) const;
    void set_vec3(const std::string& name, glm::vec3 vec) const;
    void set_vec4(const std::string& name, glm::vec4 vec) const;
    void set_mat4(const std::string& name, glm::mat4 mat) const;
    void set_int(const std::string& name, int value) const;

private:
    // Updated to take shader source string instead of file path
    void add_shader(unsigned int program, const char* shader_source, GLenum shader_type);
};