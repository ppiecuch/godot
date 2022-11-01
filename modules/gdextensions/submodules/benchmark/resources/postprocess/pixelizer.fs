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
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

// NOTE: Render size values must be passed from code
const float renderWidth = 800.0;
const float renderHeight = 450.0;

float pixelWidth = 5.0;
float pixelHeight = 5.0;

void fragment()
{
    float dx = pixelWidth*(1.0/renderWidth);
    float dy = pixelHeight*(1.0/renderHeight);

    vec2 coord = vec2(dx*floor(UV.x/dx), dy*floor(UV.y/dy));

    vec3 tc = texture(TEXTURE, coord).rgb;

    COLOR = vec4(tc, 1.0);
}
