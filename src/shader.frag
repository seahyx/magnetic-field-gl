#version 330 core

#define PIXELS_PER_METER 100

in vec4 gl_FragCoord;

out vec4 frag_color;

uniform float center_x;
uniform float center_y;
uniform float zoom;
uniform vec2 window;

// Define a struct for a magnetic dipole
struct MagneticDipole {
    vec2 position;
    vec2 direction;
    float moment;
};

// Uniform array of magnetic dipoles
uniform MagneticDipole dipoles[10]; // Support up to 10 dipoles
uniform int num_dipoles;            // Number of active dipoles

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

vec2 calculateMagneticField(vec2 pos, MagneticDipole dipole) {
    vec2 A = pos - dipole.position; // Relative position of current pixel from dipole
    float r = length(A);
    if (r == 0.0) return vec2(0.0); // Avoid divide-by-zero
    vec2 A_rad = A / r; // Radial direction of pixel from dipole
    vec2 A_tan = vec2(A_rad.y, -A_rad.x); // Clockwise tangential direction
    
    float cos_theta = dot(A_rad, dipole.direction); // Component of dipole moment in radial unit vector
    float sin_theta = dot(A_tan, dipole.direction); // Since A_tan is perpendicular to A_rad

    r /= PIXELS_PER_METER;
    float r_cube = r * r * r;
    
    // We skip the constants, assume pre-multiplied
    float B_r = (2.0 * dipole.moment * cos_theta) / r_cube; // Radial field magnitude
    float B_theta = (dipole.moment * sin_theta) / r_cube; // Tangential field magnitude

    return B_r * A_rad + B_theta * A_tan;
}

vec3 drawContour(vec3 col, float fieldStrLen, float logField)
{
    // Add field lines using contours
    float lineStrength = fieldStrLen > 0.50 ?sin(logField * 20.0) : 0.0; // Frequency controls line density
    float line = abs(lineStrength)>0.95 ? 1.0 : 0.0; // adjusting the 0.95 value will change the line density
    vec3 lineColor = vec3(1.0);
    return mix(col, lineColor, line*0.8); // Blend lines with base color (0.8 = line opacity)
}

void main() {
    vec2 fieldStr = vec2(0.0);

    // Accumulate magnetic field contributions
    for (int i = 0; i < num_dipoles; i++) {
        fieldStr += calculateMagneticField(gl_FragCoord.xy, dipoles[i]);
    }

    float fieldStrLen = length(fieldStr); // Field strength magnitude
    // float logField = log(1.0 + fieldStrLen); // Log scale for better visualization
    //for dipole shading
    float normalizedField = min(fieldStrLen*0.1, 1.0);
    // Base color from field strength
    vec3 col = hsl2rgb((1.0 - normalizedField) * (300.0 / 360.0), 1.0, min(normalizedField * 2.0, 0.5));

    // Contour shading
    // col = drawContour(col, fieldStrLen, logField);

    frag_color = vec4(col, 1.0f);
}