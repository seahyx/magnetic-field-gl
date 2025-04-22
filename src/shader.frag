#version 330 core

uniform int pixels_per_meter;

in vec4 gl_FragCoord;
out vec4 frag_color;

uniform mat4 view;
uniform mat4 projection;

// Define a struct for a magnetic dipole
struct MagneticDipole {
    vec3 position;
    vec3 direction;
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
    // Convert fragment coordinate to normalized device coordinates
    vec2 ndc = (gl_FragCoord.xy / vec2(1920, 1080)) * 2.0 - 1.0;
    float aspect_ratio = 1920.0 / 1080.0;

    // Compute world position on near plane
    mat4 inv_view = inverse(view);
    mat4 inv_proj = inverse(projection);
    vec4 near_plane = inv_proj * vec4(ndc, -1.0, 1.0);
    near_plane /= near_plane.w;
    vec4 world_near = inv_view * near_plane;
    
    // Compute ray direction
    vec3 camera_pos = (inv_view * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 ray_dir = normalize(world_near.xyz - camera_pos);

    // Cuboid bounds
    vec3 cuboid_min = vec3(-2.0, -1.5, -1.0);
    vec3 cuboid_max = vec3(2.0, 1.5, 1.0);

    // Ray-box intersection
    vec3 inv_dir = 1.0 / ray_dir;
    vec3 t0 = (cuboid_min - camera_pos) * inv_dir;
    vec3 t1 = (cuboid_max - camera_pos) * inv_dir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float t_near = max(max(tmin.x, tmin.y), tmin.z);
    float t_far = min(min(tmax.x, tmax.y), tmax.z);

    vec3 field_str = vec3(0.0);
    if (t_near < t_far && t_near >= 0.0) {
        // Intersection point
        vec3 pos = camera_pos + t_near * ray_dir;

        // Accumulate magnetic field
        for (int i = 0; i < num_dipoles; i++) {
            field_str += calculateMagneticField(pos, dipoles[i]);
        }
    }

    float field_str_len = length(field_str);
    float normalized_field = min(field_str_len * 0.1, 1.0);
    vec3 col = hsl2rgb((1.0 - normalized_field) * (300.0 / 360.0), 1.0, min(normalized_field * 2.0, 0.5));

    // Make field semi-transparent
    frag_color = vec4(col, 0.5);
}