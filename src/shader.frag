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
 
float get_iterations()
{
    // Initial z value converted from coordinates
    vec2 c = vec2((gl_FragCoord.x / window.x - 0.5) * window.x/window.y * zoom * INITIAL_ZOOM + center_x + OFFSET_CENTER_X, // real
                (gl_FragCoord.y / window.y - 0.5) * zoom * INITIAL_ZOOM + center_y); // imag

    float c2 = dot(c, c);
    // skip computation inside M1 - https://iquilezles.org/articles/mset1bulb
    if( 256.0*c2*c2 - 96.0*c2 + 32.0*c.x - 3.0 < 0.0 ) return 0.0;
    // skip computation inside M2 - https://iquilezles.org/articles/mset2bulb
    if( 16.0*(c2+2.0*c.x+1.0) - 1.0 < 0.0 ) return 0.0;

    const float B = 256.0;
 
    float n = 0; // Number of iterations
    vec2 z = vec2(0.0); // Next z value
 
    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c; // z = z^2 + c
        if (dot(z,z) > (B*B)) break;
        n += 1.0;
    }

    if (n > MAX_ITERATIONS - 1.0) return 0.0;

    return n - log2(log2(dot(z,z))) + 4.0; // optimized smooth iteration count
}

vec4 return_color()
{
    float n = get_iterations();
    vec3 col = (n < 0.5) ? vec3(0.0) : 0.5 + 0.5*cos(3.0 + n*0.15 + vec3(0.0, 0.6, 1.0));

    return vec4(col, 1.0);
}
 
void main()
{
    frag_color = return_color();
}