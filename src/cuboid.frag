#version 330 core
out vec4 frag_color;

in vec4 vertex_color; // Color from vertex shader
uniform vec4 color; // Fallback uniform color

void main()
{
    // Use vertex color if available (non-zero), otherwise use uniform color
    if (vertex_color.a > 0.0) {
        frag_color = vertex_color;
    } else {
        frag_color = color;
    }
}