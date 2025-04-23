#version 330 core
in vec3 field;
out vec4 frag_color;

uniform int use_color; // 0 for white, 1 for colored
uniform float sensitivity_scaled;

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
        float normalized_field = min(field_str_len * 0.000001 * sensitivity_scaled, 1.0);
        vec3 col = hsl2rgb((1.0 - normalized_field) * (300.0 / 360.0), 1.0, min(normalized_field * 2.0, 0.5));
        frag_color = vec4(col, 1.0f);
    } else {
        // Plain white
        frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}