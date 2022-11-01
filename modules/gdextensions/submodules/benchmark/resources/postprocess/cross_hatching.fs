shader_type canvas_item;

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
#else
 precision mediump float;
#endif
#endif

// Input vertex attributes (from vertex shader)
varying vec4 fragColor;

// Input uniform values
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

float hatchOffsetY = 5.0;
float lumThreshold01 = 0.9;
float lumThreshold02 = 0.7;
float lumThreshold03 = 0.5;
float lumThreshold04 = 0.3;

void fragment()
{
    vec3 tc = vec3(1.0, 1.0, 1.0);
    float lum = length(texture(TEXTURE, UV).rgb);

    if (lum < lumThreshold01)
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold02)
    {
        if (mod(gl_FragCoord .x - gl_FragCoord .y, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold03)
    {
        if (mod(gl_FragCoord .x + gl_FragCoord .y - hatchOffsetY, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold04)
    {
        if (mod(gl_FragCoord .x - gl_FragCoord .y - hatchOffsetY, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    COLOR = vec4(tc, 1.0);
}
