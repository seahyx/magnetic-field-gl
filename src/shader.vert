#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 view;
uniform mat4 projection;
out vec3 world_pos; // Pass world position to fragment shader

void main()
{
    gl_Position = projection * view * vec4(pos, 1.0);
    world_pos = pos; // Vertex position is already in world space
}