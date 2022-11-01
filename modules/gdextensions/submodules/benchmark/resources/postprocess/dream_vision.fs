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

void fragment()
{
    vec4 color = texture(TEXTURE, UV);

    color += texture(TEXTURE, UV + 0.001);
    color += texture(TEXTURE, UV + 0.003);
    color += texture(TEXTURE, UV + 0.005);
    color += texture(TEXTURE, UV + 0.007);
    color += texture(TEXTURE, UV + 0.009);
    color += texture(TEXTURE, UV + 0.011);

    color += texture(TEXTURE, UV - 0.001);
    color += texture(TEXTURE, UV - 0.003);
    color += texture(TEXTURE, UV - 0.005);
    color += texture(TEXTURE, UV - 0.007);
    color += texture(TEXTURE, UV - 0.009);
    color += texture(TEXTURE, UV - 0.011);

    color.rgb = vec3((color.r + color.g + color.b)/3.0);
    color = color/9.5;

    COLOR = color;
}
