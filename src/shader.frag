#version 330 core
 
#define MAX_ITERATIONS 512
#define OFFSET_CENTER_X -0.75
#define INITIAL_ZOOM 3.0

in vec4 gl_FragCoord;

out float gl_FragDepth;
out vec4 frag_color;

uniform float center_x;
uniform float center_y;
uniform float zoom;
uniform vec2 window;

vec2 calculateMagneticField(vec2 pos, vec2 source, vec2 direction, float moment)
{
    vec2 A = pos - source;
    vec2 A_rad = normalize(A);
    vec2 A_tan = vec2(A_rad.y, -A_rad.x); // Clockwise tangent
    direction = normalize(direction);
    float r = length(A);
    float r_cube = pow(r, 3);
    float theta = acos(dot(A, direction) / r);

    // We skip the constants, assume pre-multiplied
    float B_r = (2 * moment * cos(theta)) / r_cube; // Radial field vector
    float B_theta = (moment *  sin(theta)) / r_cube; // Tangential field vector

    return B_r * A_rad + B_theta * A_tan;
}
 
void main()
{
    vec2 fieldStr = calculateMagneticField(gl_FragCoord.xy, vec2(0.0f), window.xy / 2.0f, 1.0f);
    frag_color = vec4(vec3(sin(length(fieldStr))), 1.0f);
}