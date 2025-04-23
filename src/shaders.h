#pragma once

const char* shader_vert = R"(
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
)";

const char* shader_frag = R"(
#version 330 core

in vec3 world_pos; // Interpolated world position from vertex shader
out vec4 frag_color;

uniform float pixels_per_meter;
uniform vec2 resolution;
uniform float plane_opacity;
uniform float sensitivity_scaled;

// Magnetic dipole struct
struct MagneticDipole {
    vec3 position;
    vec3 direction;
    float moment;
};

// Use UBO in a uniform array to pass in large sets of dipole data
layout (std140) uniform DipoleBuffer {
    MagneticDipole dipoles[1024]; // Up to 1024 dipoles
};
uniform int num_dipoles;

float hue2rgb(float f1, float f2, float hue) {
    if (hue < 0.0)
        hue += 1.0;
    else if (hue > 1.0)
        hue -= 1.0;
    float res;
    if ((6.0 * hue) < 1.0)
        res = f1 + (f2 - f1) * 6.0 * hue;
    else if ((2.0 * hue) < 1.0)
        res = f2;
    else if ((3.0 * hue) < 2.0)
        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    else
        res = f1;
    return res;
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb;
    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z); // Luminance
    } else {
        float f2;
        if (hsl.z < 0.5)
            f2 = hsl.z * (1.0 + hsl.y);
        else
            f2 = hsl.z + hsl.y - hsl.y * hsl.z;
        float f1 = 2.0 * hsl.z - f2;
        rgb.r = hue2rgb(f1, f2, hsl.x + (1.0 / 3.0));
        rgb.g = hue2rgb(f1, f2, hsl.x);
        rgb.b = hue2rgb(f1, f2, hsl.x - (1.0 / 3.0));
    }
    return rgb;
}

vec3 hsl2rgb(float h, float s, float l) {
    return hsl2rgb(vec3(h, s, l));
}

vec3 calculateMagneticField(vec3 pos, MagneticDipole dipole) {
    vec3 A = pos - dipole.position;
    float r = length(A);
    if (r < 0.0001) return vec3(0.0); // Avoid divide-by-zero
    vec3 A_rad = A / r;

    // Find a perpendicular axis for tangential plane
    vec3 perpAxis;
    if (abs(A_rad.x) < abs(A_rad.y) && abs(A_rad.x) < abs(A_rad.z)) {
        perpAxis = vec3(1.0, 0.0, 0.0);
    } else if (abs(A_rad.y) < abs(A_rad.z)) {
        perpAxis = vec3(0.0, 1.0, 0.0);
    } else {
        perpAxis = vec3(0.0, 0.0, 1.0);
    }

    vec3 A_tan1 = normalize(cross(A_rad, perpAxis));
    vec3 A_tan2 = cross(A_rad, A_tan1);

    float cos_theta = dot(A_rad, dipole.direction);
    vec3 dirTangential = dipole.direction - cos_theta * A_rad;
    float sin_theta_mag = length(dirTangential);

    float r_meters = r / pixels_per_meter;
    float r_cube = r_meters * r_meters * r_meters;

    float B_r = (2.0 * dipole.moment * cos_theta) / r_cube;
    vec3 B_field = B_r * A_rad;

    if (sin_theta_mag > 0.0001) {
        vec3 A_tan_dir = dirTangential / sin_theta_mag;
        float B_theta = (dipole.moment * sin_theta_mag) / r_cube;
        B_field += B_theta * A_tan_dir;
    }

    return B_field;
}

void main() {
    // Calculate total magnetic field at world_pos
    vec3 field_str = vec3(0.0);
    for (int i = 0; i < num_dipoles; i++) {
        field_str += calculateMagneticField(world_pos, dipoles[i]);
    }

    // Map field strength to color
    float field_str_len = length(field_str);
    float normalized_field = min(field_str_len * 0.000001 * sensitivity_scaled, 1.0);
    vec3 col = hsl2rgb((1.0 - normalized_field) * (300.0 / 360.0), 1.0, min(normalized_field * 2.0, 0.5));
    frag_color = vec4(col, plane_opacity);
}
)";

const char* cuboid_vert = R"(
#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
}
)";

const char* cuboid_frag = R"(
#version 330 core
out vec4 frag_color;

uniform vec4 color;

void main()
{
    frag_color = color;
}
)";

const char* field_line_vert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aField;

out vec3 field;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    field = aField;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* field_line_frag = R"(
#version 330 core
in vec3 field;
out vec4 frag_color;

uniform int use_color; // 0 for white, 1 for colored

float hue2rgb(float f1, float f2, float hue) {
    if (hue < 0.0)
        hue += 1.0;
    else if (hue > 1.0)
        hue -= 1.0;
    float res;
    if ((6.0 * hue) < 1.0)
        res = f1 + (f2 - f1) * 6.0 * hue;
    else if ((2.0 * hue) < 1.0)
        res = f2;
    else if ((3.0 * hue) < 2.0)
        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    else
        res = f1;
    return res;
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb;
    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z); // Luminance
    } else {
        float f2;
        if (hsl.z < 0.5)
            f2 = hsl.z * (1.0 + hsl.y);
        else
            f2 = hsl.z + hsl.y - hsl.y * hsl.z;
        float f1 = 2.0 * hsl.z - f2;
        rgb.r = hue2rgb(f1, f2, hsl.x + (1.0 / 3.0));
        rgb.g = hue2rgb(f1, f2, hsl.x);
        rgb.b = hue2rgb(f1, f2, hsl.x - (1.0 / 3.0));
    }
    return rgb;
}

vec3 hsl2rgb(float h, float s, float l) {
    return hsl2rgb(vec3(h, s, l));
}

void main()
{
    if (use_color == 1) {
        // Existing color mapping (blue for weak, red for strong)
        float field_str_len = length(field);
        float normalized_field = min(field_str_len * 0.000001, 1.0);
        vec3 col = hsl2rgb((1.0 - normalized_field) * (300.0 / 360.0), 1.0, min(normalized_field * 2.0, 0.5));
        frag_color = vec4(col, 1.0f);
    } else {
        // Plain white
        frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
)";