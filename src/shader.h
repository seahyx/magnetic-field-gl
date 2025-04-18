#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/type_ptr.hpp>

class Shader
{
public:
    unsigned int program_ID;
    Shader(const char* vertex_shader_path, const char* fragment_shader_path);
    ~Shader();

    void use_shader();

    void set_float(const std::string& name, float value) const;
    void set_vec2(const std::string& name, glm::vec2 vec) const;
    void set_vec3(const std::string& name, glm::vec3 vec) const;
    void set_vec4(const std::string& name, glm::vec4 vec) const;
    void set_int(const std::string& name, int value) const;

private:
    std::string read_shader_file(const char* file_path);
    void add_shader(unsigned int program, const char* shader_path, GLenum shader_type);
};