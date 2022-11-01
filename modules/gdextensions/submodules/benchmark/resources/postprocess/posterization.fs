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

float gamma = 0.6;
float numColors = 8.0;

void fragment()
{
    vec3 color = texture(TEXTURE, UV).rgb;

    color = pow(color, vec3(gamma, gamma, gamma));
    color = color*numColors;
    color = floor(color);
    color = color/numColors;
    color = pow(color, vec3(1.0/gamma));

    COLOR = vec4(color, 1.0);
}
