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

void fragment()
{
    vec3 color = texture(TEXTURE, UV).rgb;
    vec3 colors[3];
    colors[0] = vec3(0.0, 0.0, 1.0);
    colors[1] = vec3(1.0, 1.0, 0.0);
    colors[2] = vec3(1.0, 0.0, 0.0);

    float lum = (color.r + color.g + color.b)/3.0;

    vec3 tc = vec3(0.0, 0.0, 0.0);

    if (lum < 0.5) tc = mix(colors[0], colors[1], lum/0.5);
    else tc = mix(colors[1], colors[2], (lum - 0.5)/0.5);

    COLOR = vec4(tc, 1.0);
}
