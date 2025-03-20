#version 330 core

#define PIXELS_PER_METER 100

in vec4 gl_FragCoord;

out float gl_FragDepth;
out vec4 frag_color;

uniform float center_x;
uniform float center_y;
uniform float zoom;
uniform vec2 window;

uniform vec2 sel_pos;
uniform vec2 sel_dir;

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
        
        rgb.r = hue2rgb(f1, f2, hsl.x + (1.0/3.0));
        rgb.g = hue2rgb(f1, f2, hsl.x);
        rgb.b = hue2rgb(f1, f2, hsl.x - (1.0/3.0));
    }   
    return rgb;
}

vec3 hsl2rgb(float h, float s, float l) {
    return hsl2rgb(vec3(h, s, l));
}

vec2 calculateMagneticField(vec2 pos, vec2 source, vec2 direction, float moment)
{
    vec2 A = pos - source;
    vec2 A_rad = normalize(A);
    vec2 A_tan = vec2(A_rad.y, -A_rad.x); // Clockwise tangent
    direction = normalize(direction);
    float r = length(A);
    float theta = acos(dot(A, direction) / r);
    r /= PIXELS_PER_METER;
    float r_cube = pow(r, 3);

    // We skip the constants, assume pre-multiplied
    float B_r = (2 * moment * cos(theta)) / r_cube; // Radial field vector
    float B_theta = (moment *  sin(theta)) / r_cube; // Tangential field vector

    return B_r * A_rad + B_theta * A_tan;
}

void main()
{
    vec2 fieldStr = calculateMagneticField(gl_FragCoord.xy, window / 2, vec2(0.0f, 1.0f) , 0.1f);
    fieldStr += calculateMagneticField(gl_FragCoord.xy, sel_pos, sel_dir , 0.1f);

    float fieldStrLen = min(length(fieldStr), 1.0f);
    vec3 col = hsl2rgb((1.0f - fieldStrLen) * (300.0f/360.0f), 1.0f, min(fieldStrLen * 2, 0.5f));
    
    frag_color = vec4(col, 1.0f);
}